#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "utils.c"


#define SAMPLE_RATE 44100


int main(int argc, char *argv[])
{
#ifndef ENABLE_7th_DSP
    return 0;
#endif


    char *fname = "7th.pcm";
    FILE* file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    int sfid = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfid, 0, 0);
    fluid_synth_set_interp_method(synth, 0, FLUID_INTERP_7THORDER);

    int notes[] = {NN3, NN6, NN1+12, NN7, NN6, NN1+12, NN6, NN7, NN6, NN4, NN5, NN3};
                //    ,NN3, NN6, NN1+12, NN7, NN6, NN1+12, NN6, NN7, NN6, NN3, NN3-1, NN2};
    float dura[] =  {0.5, 0.5, 0.5,   0.5,  0.5, 0.5,    0.5, 0.5, 0.5, 0.5, 0.5, 1.5};
                    // ,0.5, 0.5, 0.5,   0.5,  0.5, 0.5,    0.5, 0.5, 0.5, 0.5, 0.5, 1.5};

    for(int i=0; i<sizeof(notes)/sizeof(int); i++){
        int frame = dura[i]*SAMPLE_RATE;
        // printf("%d-%f, \t%d\n",notes[i],dura[i], frame);
        fluid_synth_noteon(synth, 0, notes[i], 127);
        int16_t buffer[frame];
        fluid_synth_write_s16_mono(synth, frame, buffer);
        fwrite(buffer, sizeof(int16_t), frame, file);
        fluid_synth_noteoff(synth, 0, notes[i]);
    }

    fclose(file);
    delete_fluid_synth(synth);

    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i 7th.pcm  7th.wav >nul 2>&1");

    return 0;
}

