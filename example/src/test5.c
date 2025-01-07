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
#define NUM_FRAMES 4410                    // SAMPLE_RATE*DURATION
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

	assert(SAMPLE_SIZE==2);
	assert(sizeof(signed short) == 2);
	assert(sizeof(int16_t) == 2);
	assert(sizeof(float) == 4);

		
    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 3);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, C, 127);
	fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
    fluid_synth_noteoff(synth, 0, C);


    fluid_synth_t *synth2 = new_fluid_synth(settings);
    int sfont2 = fluid_synth_sfload(synth2, argv[1], 1);
    fluid_synth_program_select(synth2, 0, sfont2, 0, 0);
    int16_t *buffer2 = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    fluid_synth_noteon(synth2, 0, C, 127);
	fluid_synth_write_u12_mono(synth2, NUM_FRAMES, buffer2);
    fluid_synth_noteoff(synth2, 0, C);

	char *p1 = (char *)buffer;
	unsigned char *p2 = (unsigned char *)buffer2;	

	for(int i=0; i<NUM_SAMPLES*2; i+=2){
		int16_t v1 = (p1[i+1] << 8) | (p1[i] & 0xFF);
		// if(v1 != p1[i+1]*256 + p1[i]){
		// 	printf("i%d: %d,%d,  %d %d \n", i,p1[i+1],p1[i], v1, p1[i+1]*256 + p1[i]);
		// }
		assert(v1 == buffer[i/2]);

		uint16_t v2 = p2[i+1]*256 + p2[i];
		assert(v2 ==  (p2[i+1] << 8) | (p2[i] & 0xFF));
		assert(v2 == buffer2[i/2]);

		int16_t v12 = v1; //>>4;
		uint16_t uv12 = v12+2048;
		//printf("char: %d  %d %d \n", v1, uv12, v2);
		assert(uv12 == v2);
	}

	for(int i=0; i<NUM_SAMPLES; i++){
		int16_t v1 = buffer[i];
		int16_t v12 = v1; //>>4;
		uint16_t uv12 = v12+2048;
		assert(uv12 == buffer2[i]);
		//printf("i%d, %d \t %d\n", i, buffer[i], buffer2[i]);
	}



    free(buffer);
	free(buffer2);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}

