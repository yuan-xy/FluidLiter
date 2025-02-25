#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"

#define SAMPLE_RATE 48000

int main(int argc, char *argv[])
{
    bool is_8bit=true;
    if(argc>1) is_8bit=false;

    char *fname = "song8_48000.pcm";
    if(!is_8bit) fname = "song12_48000.pcm";

    FILE* file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH(.sample_rate=48000.0f);
    // fluid_synth_set_sample_rate(synth, 48000.0f);
    assert(synth->sample_rate == 48000);//break fluid_voice.c:1032

    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

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
        system("ffmpeg -hide_banner -y -f u8 -ar 48000 -ac 1 -i song8_48000.pcm -acodec pcm_u8 song8_48000.wav");
    }else{
        system("ffmpeg -hide_banner -y -f s16le -ar 48000 -ac 1 -i song12_48000.pcm song12_48000.wav");
        // system("ffmpeg -hide_banner -f s16le -ar 48000 -ac 1 -i song12_48000.pcm -filter:a 'volume=16' song12_48000.wav");
        //虽然我认为是u12格式，ffmepg并不支持，所以当成了s16le格式。也就导致wav声音偏小, 手动放大16倍, 音量放大后有失真。
    }

    return 0;
}

