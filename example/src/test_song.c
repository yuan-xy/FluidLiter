#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidlite.h"
#include "fluid_synth.h"
#include "utils.c"


#define SAMPLE_RATE 44100


int main(int argc, char *argv[])
{
    bool is_8bit=true;
    if(argc>1) is_8bit=false;

    char *fname = "song8.pcm";
    if(!is_8bit) fname = "song12.pcm";

    FILE* file = fopen(fname, "wb");

    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 3);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    // fluid_synth_set_gen(synth, 0, GEN_VIBLFODELAY, 100);    // 延迟时间为 100 毫秒
    // fluid_synth_set_gen(synth, 0, GEN_VIBLFOFREQ, 6);       // LFO 频率为 6 Hz
    // fluid_synth_set_gen(synth, 0, GEN_VIBLFOTOPITCH, 200);  // 调制深度为 200 音分

    //两者的效果似乎是一样的：modLfoToPitch 和 vibLfoToPitch
    fluid_synth_set_gen(synth, 0, GEN_MODLFODELAY, 100);    // 延迟时间为 100 毫秒
    fluid_synth_set_gen(synth, 0, GEN_MODLFOFREQ, 6);       // LFO 频率为 6 Hz


    fluid_synth_set_gen(synth, 0, GEN_MODLFOTOPITCH, 0);  // 调制深度为 200 音分
    // fluid_synth_set_gen和fluid_synth_cc实现同样的效果？
    fluid_synth_cc(synth, 0, 1, 127); // CC 1（Modulation Wheel）通常映射到 GEN_MODLFOTOPITCH
    //从源代码看fluid_synth_cc首先会在cc保存设置的值：chan->cc[num] = value;
    //最终会调用fluid_synth_modulate_voices，也就是默认有noteon，才有voice。
    //所以大概fluid_synth_set_gen是初始配置，fluid_synth_cc是播放中设置。

    print_cc_values(synth, 0);


    // float attack_time = fluid_synth_get_gen(synth, 0, GEN_VOLENVATTACK);
    // printf("GEN_VOLENVATTACK: %f\n", attack_time);
    // fluid_synth_set_gen(synth, 0, GEN_VOLENVATTACK, 300); // 设置起音时间为 300 毫秒，似乎没效果

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
    delete_fluid_settings(settings);

    if(is_8bit){
        system("ffmpeg -hide_banner -f u8 -ar 44100 -ac 1 -i song8.pcm -acodec pcm_u8 song8.wav");
    }else{
        system("ffmpeg -hide_banner -f s16le -ar 44100 -ac 1 -i song12.pcm song12.wav");
        // system("ffmpeg -hide_banner -f s16le -ar 44100 -ac 1 -i song12.pcm -filter:a 'volume=16' song12.wav");
        //虽然我认为是u12格式，ffmepg并不支持，所以当成了s16le格式。也就导致wav声音偏小, 手动放大16倍。
    }

    return 0;
}
