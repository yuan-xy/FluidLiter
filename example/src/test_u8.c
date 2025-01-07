#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidlite.h"
#include "fluid_synth.h"

#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(int16_t)         // s16
#define NUM_FRAMES 441                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)


int main(int argc, char *argv[])
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 3);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, C, 127);
	fluid_synth_write_u12_mono(synth, NUM_FRAMES, buffer);
    fluid_synth_noteoff(synth, 0, C);


    fluid_synth_t *synth2 = new_fluid_synth(settings);
    int sfont2 = fluid_synth_sfload(synth2, argv[1], 1);
    fluid_synth_program_select(synth2, 0, sfont2, 0, 0);

    uint8_t *buffer2 = calloc(sizeof(uint8_t) , NUM_SAMPLES);

    fluid_synth_noteon(synth2, 0, C, 127);
	fluid_synth_write_u8_mono(synth2, NUM_FRAMES, buffer2);
    fluid_synth_noteoff(synth2, 0, C);

	for(int i=0; i<NUM_SAMPLES; i++){
		uint16_t v1 = buffer[i];
        printf("i%d: %d, %d, %d\n", i, v1, v1>>4, buffer2[i]);
		//assert(v1>>4 == buffer2[i]);
	}

    free(buffer);
	free(buffer2);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}

