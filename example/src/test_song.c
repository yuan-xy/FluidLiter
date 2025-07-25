#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"
#include "utils.c"
#include "fluid_sfont.h"


#define SAMPLE_RATE 44100
#define INT_MAX16 65535


int compare_binary_files(const char *file1, const char *file2, bool out_sum) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");

    if (f1 == NULL || f2 == NULL) {
        perror("无法打开文件");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return INT_MAX16; // 文件打开失败，返回最大值
    }

    // 检查文件长度
    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);
    long len1 = ftell(f1);
    long len2 = ftell(f2);

    if (len1 != len2) {
        fclose(f1);
        fclose(f2);
        return INT_MAX16; // 文件长度不相等，返回最大值
    }

    // 重新定位到文件开头
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);

    int sum_of_differences = 0;
    int count_of_diff = 0;
    int pos = 0;
    unsigned char byte1, byte2;

    // 逐字节比较
    while (fread(&byte1, 1, 1, f1) == 1 && fread(&byte2, 1, 1, f2) == 1) {
        pos += 1;
        if (byte1 != byte2) {
            sum_of_differences += abs(byte1 - byte2); // 计算差异值的绝对值并累加
            count_of_diff += 1;
            printf("pos:%d,  %d - %d\n", pos, byte1, byte2);
        }
    }

    fclose(f1);
    fclose(f2);
    if(sum_of_differences > 0){
        printf("sum_of_differences:%d, count_of_diff:%d\n", sum_of_differences, count_of_diff);
    }
    if(out_sum){
        return sum_of_differences;
    }else{
        return sum_of_differences-count_of_diff;
    }
    
}


int main(int argc, char *argv[])
{
    bool is_8bit=true;
    if(argc>1) is_8bit=false;

    char *fname = "song8.pcm";
    if(!is_8bit) fname = "song12.pcm";

    FILE* file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH(.with_reverb=false);
    printf("sizeof(fluid_voice_t)=%u\n", sizeof(fluid_voice_t));
    //double:assert(2712 == sizeof(fluid_voice_t));
    //float: sizeof(fluid_voice_t)=1752   
    printf("sizeof(fluid_mod_t)=%u\n", sizeof(fluid_mod_t)); //24 on x64, 16 on x86
    
    int sfid = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfid, 0, 0);

    //颤音效果（Vibrato）
    // fluid_synth_set_gen的实现很奇怪，很难用，建议直接调用fluid_channel_set_gen
    // 下面的代码并不能正确运行，实际结果如下：
        // GEN_6: GEN_VIBLFOTOPITCH, value=4788000.000000
        // GEN_23: GEN_VIBLFODELAY, value=1688000.000000
        // GEN_24: GEN_VIBLFOFREQ, value=107000.000000
    // fluid_synth_set_gen(synth, 0, GEN_VIBLFODELAY, 100);    // 延迟时间为 100 毫秒
    // fluid_synth_set_gen(synth, 0, GEN_VIBLFOFREQ, 6);       // LFO 频率为 6 Hz
    // fluid_synth_set_gen(synth, 0, GEN_VIBLFOTOPITCH, 200);  // 调制深度为 200 音分

    //两者的效果似乎是一样的：modLfoToPitch 和 vibLfoToPitch
    // fluid_channel_set_gen(synth->channel[0], GEN_MODLFODELAY, 100.0f);    // 延迟时间为 100 毫秒
    // fluid_channel_set_gen(synth->channel[0], GEN_MODLFOFREQ, 6.0f);       // LFO 频率为 6 Hz
    // fluid_channel_set_gen(synth->channel[0], GEN_MODLFOTOPITCH, 200.0f);  // 调制深度为 200 音分

    // fluid_synth_set_gen和fluid_synth_cc实现同样的效果？
    fluid_synth_cc(synth, 0, 1, 127); // CC 1（Modulation Wheel）通常映射到 GEN_MODLFOTOPITCH
    //从源代码看fluid_synth_cc首先会在cc保存设置的值：chan->cc[num] = value;
    //最终会调用fluid_synth_modulate_voices，也就是默认有noteon，才有voice。
    //所以大概fluid_synth_set_gen是初始配置，fluid_synth_cc是播放中设置。
    //还有一个关键区别，cc是midi规范里的，gen是soundfont规范里的。

    //震音效果（Tremolo）
    // fluid_synth_set_gen(synth, 0, GEN_MODLFOFREQ, 5.0f);  // 5 Hz 的震音速度
    // fluid_synth_set_gen(synth, 0, GEN_MODLFOTOVOL, 500.0f);  // 调制深度为 500

    print_cc_values(synth, 0);
    print_gen_values(synth, 0, false);

    float attack_time = fluid_synth_get_gen(synth, 0, GEN_VOLENVATTACK);
    printf("GEN_VOLENVATTACK: %f\n", attack_time);
    // fluid_synth_set_gen(synth, 0, GEN_VOLENVATTACK, 300); // 设置起音时间为 300 毫秒，似乎没效果, 因为300不代表300ms

    int notes[] = {NN3, NN6, NN1+12, NN7, NN6, NN1+12, NN6, NN7, NN6, NN4, NN5, NN3};
                //    ,NN3, NN6, NN1+12, NN7, NN6, NN1+12, NN6, NN7, NN6, NN3, NN3-1, NN2};
    float dura[] =  {0.5, 0.5, 0.5,   0.5,  0.5, 0.5,    0.5, 0.5, 0.5, 0.5, 0.5, 1.5};
                    // ,0.5, 0.5, 0.5,   0.5,  0.5, 0.5,    0.5, 0.5, 0.5, 0.5, 0.5, 1.5};

    for(int i=0; i<sizeof(notes)/sizeof(int); i++){
        int frame = dura[i]*SAMPLE_RATE;
        // printf("%d-%f, \t%d\n",notes[i],dura[i], frame);
        fluid_synth_noteon(synth, 0, notes[i], 127);
        if(is_8bit){
            uint8_t buffer[frame];
            fluid_synth_write_u8_mono(synth, frame, buffer);
            fwrite(buffer, sizeof(uint8_t), frame, file);
        }else{
            uint16_t buffer[frame];
            fluid_synth_write_u12_mono(synth, frame, buffer);
            fwrite(buffer, sizeof(uint16_t), frame, file);
        }
        fluid_synth_noteoff(synth, 0, notes[i]);
    }

    fclose(file);
    delete_fluid_synth(synth);

    if(is_8bit){
#ifdef __linux__
        int ret;
        if(sizeof(fluid_real_t) == 4){
            ret = compare_binary_files("song8.pcm", "example/song8.pcm", true);
            assert(ret <= 3);
        }else{
            ret = compare_binary_files("song8.pcm", "example/song8.pcm", false);
            assert(ret == 0);
        }
#endif
        system("ffmpeg -hide_banner -y -f u8 -ar 44100 -ac 1 -i song8.pcm -acodec pcm_u8 song8.wav >nul 2>&1");
    }else{
        //system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i song12.pcm song12.wav");
        system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i song12.pcm -filter:a 'volume=16' song12.wav >nul 2>&1");
        //虽然我认为是u12格式，ffmepg并不支持，所以当成了s16le格式。也就导致wav声音偏小, 手动放大16倍。
    }

    return 0;
}

