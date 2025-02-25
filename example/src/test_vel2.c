#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"
#include "misc.h"
#include <float.h> // 用于定义 FLT_MIN

#define RED_TEXT(x) "\033[31m" #x "\033[0m"


#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 441                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)




int main(int argc, char *argv[])
{
    printf("sizeof(fluid_real_t): %u\n", sizeof(fluid_real_t));
#ifdef WITH_FLOAT
    assert(sizeof(fluid_real_t)==4);
#endif
    set_log_level(FLUID_WARN);
    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    printf("synth.gain:%f\n", synth->gain);
    assert(float_eq(0.4f, synth->gain));

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    float pre_mean_db = -FLT_MAX;
    float pre_peak_db = -FLT_MAX;

	for(int i=10; i<=120; i+=10){
        fluid_synth_noteon(synth, 0, NOTE_C, i);
        fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 2, buffer, 1, 2);

        float cur_mean_db = calculateVolumeDB(buffer, NUM_SAMPLES);
        float cur_peak_db = calculate_peak_dB(buffer, NUM_SAMPLES);

        printf(RED_TEXT(Velocity) " %d:\tMEAN DB %.1f  ,\tPEAK DB:%.1f \n", 
            i, cur_mean_db, cur_peak_db);

        assert(cur_mean_db > pre_mean_db || pre_mean_db > -33);
        assert(cur_peak_db > pre_peak_db || pre_peak_db > -33);
        // 110很反常
        // Velocity 100:   MEAN DB -31.0  ,        PEAK DB:-24.8
        // Velocity 110:   MEAN DB -31.3  ,        PEAK DB:-25.3

        assert(cur_peak_db > cur_mean_db);


        pre_mean_db = cur_mean_db;
        pre_peak_db = cur_peak_db;

		for(int j=0; j<10; j++){
			fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 2, buffer, 1, 2);
			printf("Velocity %d:\tMEAN DB %.1f  ,\tPEAK DB:%.1f\n", 
				i,  calculateVolumeDB(buffer, NUM_SAMPLES), calculate_peak_dB(buffer, NUM_SAMPLES));
		}

    }


    free(buffer);
    delete_fluid_synth(synth);
    return 0;
}

