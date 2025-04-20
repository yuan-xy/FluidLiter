#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"

#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 44100                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)


int main(int argc, char *argv[])
{
    char *fname = "test_multi_sfont.pcm";
    FILE* file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, NOTE_C, 127);
	fluid_synth_write_s16_mono(synth, NUM_FRAMES, (uint16_t *)buffer);
    fluid_synth_noteoff(synth, 0, NOTE_C);
    fwrite(buffer, sizeof(uint16_t), NUM_FRAMES, file);


    int sfont2 = fluid_synth_sfload(synth, "example/sf_/Boomwhacker.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont2, 0, 0);
    
    fluid_synth_noteon(synth, 0, NOTE_C, 127);
	fluid_synth_write_s16_mono(synth, NUM_FRAMES, (uint16_t *)buffer);
    fluid_synth_noteoff(synth, 0, NOTE_C);
    fwrite(buffer, sizeof(uint16_t), NUM_FRAMES, file);

    fluid_synth_program_select(synth, 0, sfont, 0, 0);
    fluid_synth_noteon(synth, 0, NOTE_D, 127);
	fluid_synth_write_s16_mono(synth, NUM_FRAMES, (uint16_t *)buffer);
    fluid_synth_noteoff(synth, 0, NOTE_D);
    fwrite(buffer, sizeof(uint16_t), NUM_FRAMES, file);

    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i test_multi_sfont.pcm test_multi_sfont.wav >nul 2>&1");


    free(buffer);
    delete_fluid_synth(synth);

    return 0;
}

