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
#define SAMPLE_SIZE sizeof(int16_t) //s16
#define DURATION 0.1 //second
#define NUM_FRAMES SAMPLE_RATE*DURATION
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

#define POLYPHONY 8

uint32_t sample_duration_us(void){
	return (uint32_t)MIRCO_SECOND/SAMPLE_RATE*NUM_FRAMES;
}

uint32_t sample_single_us(void){
	return (uint32_t)MIRCO_SECOND/SAMPLE_RATE;
//	return (uint32_t)sample_single_us()/NUM_FRAMES;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    fluid_settings_t* settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); //在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", POLYPHONY); 
    fluid_settings_setint(settings, "synth.midi-channels", 1); 
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);
    FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;

    void play_seconds(int value, int seconds){
    	fluid_synth_noteon(synth, 0, value, 127);
		int j=0;
		for(; j <= seconds*MIRCO_SECOND/sample_duration_us(); j++){
			fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
			fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);
		}
		printf("value:%d, j:%d\n", value, j);
		fluid_synth_noteoff(synth, 0, value);
    }

    for(int i=-2; i<3; i++){
        int notes[] = {C, D, E, F, G, A, B};
        int note_count = sizeof(notes) / sizeof(notes[0]);
        for (int j = 0; j<note_count; j++) {
            int v = notes[j];
            v = v + i * 12;
            printf("note value:%d\n", v);
            play_seconds(v, 2);
        }
    }

	// for(int i=36; i<=96; i+=2){
	// 	fluid_synth_noteon(synth, 0, i, 127);
	// 	int j=0;
	// 	for(; j <= MIRCO_SECOND/sample_duration_us(); j++){
	// 		fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
	// 		fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);
	// 	}
	// 	printf("i:%d, j:%d\n", i, j);
	// 	fluid_synth_noteoff(synth, 0, i);
	// }

    fclose(file);
    free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}