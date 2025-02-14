#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"
#include "misc.h"
#include <float.h> // 用于定义 FLT_MIN

#define RED_TEXT(x) "\033[31m" #x "\033[0m"


#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 44100                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)




int main(int argc, char *argv[])
{
    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    printf("synth.gain:%f\n", synth->gain);
    assert(float_eq(0.4f, synth->gain));

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    float pre_mean_db = -FLT_MAX;
    float pre_peak_db = -FLT_MAX;
    float pre_peak_db_1024 = -FLT_MAX;

	for(int i=10; i<=120; i+=10){
        fluid_synth_noteon(synth, 0, NOTE_C, i);
        fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 2, buffer, 1, 2);

        float cur_mean_db = calculateVolumeDB(buffer, NUM_SAMPLES);
        float cur_peak_db = calculate_peak_dB(buffer, NUM_SAMPLES);
        float cur_peak_db_1024 = calculate_peak_dB_1024(buffer, NUM_SAMPLES);
        printf(RED_TEXT(Velocity) " %d:\tMEAN DB %.1f  ,\tPEAK DB:%.1f ,\tPEAK1024 DB:%.1f\n", 
            i, cur_mean_db, cur_peak_db, cur_peak_db_1024);

        assert(cur_mean_db > pre_mean_db);
        assert(cur_peak_db > pre_peak_db);
        assert(cur_peak_db_1024 > pre_peak_db_1024);

        assert(cur_peak_db > cur_peak_db_1024);
        assert(cur_peak_db_1024 > cur_mean_db);

        pre_mean_db = cur_mean_db;
        pre_peak_db = cur_peak_db;
        pre_peak_db_1024 = cur_peak_db_1024;

        if(false){
            char *fname = "velocity.pcm";
            FILE* file = fopen(fname, "wb");
            fwrite(buffer, sizeof(uint16_t), NUM_SAMPLES, file);
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i velocity.pcm velocity%d.wav", i);
            system(cmd);
        }
    }


    free(buffer);
    delete_fluid_synth(synth);
    return 0;
}

