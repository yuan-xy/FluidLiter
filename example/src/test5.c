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


void set_bit(uint8_t* value, uint8_t index){
	uint8_t bitmask = 1 << index;
	*value |= bitmask;
}

void unset_bit(uint8_t* value, uint8_t index){
	uint8_t bitmask = ~(1 << index);
	*value &= bitmask;
}

void change_bit(uint8_t* value, uint8_t index, uint8_t bit){
	if(bit) set_bit(value, index);
	else unset_bit(value, index);
}



int main(int argc, char *argv[])
{
	uint8_t cstate=0;
	uint8_t pstate=0xFF;
	change_bit(&cstate, 1, 1);
	change_bit(&pstate, 2, 0);
	set_bit(&cstate, 3);
	unset_bit(&pstate, 4);
	assert(cstate == 0b00001010);
	assert(pstate == 0b11101011);


    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 3);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, C, 80);
	fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
    fluid_synth_noteoff(synth, 0, C);

	for(int i=0; i<NUM_SAMPLES*2; i++){
		assert(SAMPLE_SIZE==2);
		int16_t v = (buffer[i+1] << 8) | (buffer[i] & 0xFF);
		int16_t v12 = v>>4;
		uint16_t uv12 = v12+2048;
		printf("v,v12,uv12: %d,\t%d,\t%d \n", v, v12, uv12);
	}

    free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}

