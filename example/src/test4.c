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


uint32_t sample_duration_us(void)
{
    return (uint32_t)MIRCO_SECOND / SAMPLE_RATE * NUM_FRAMES;
}

uint32_t sample_single_us(void)
{
    return (uint32_t)MIRCO_SECOND / SAMPLE_RATE;
    //	return (uint32_t)sample_single_us()/NUM_FRAMES;
}

int main(int argc, char *argv[])
{
	char *filename = "example/sf_/GMGSx_1.sf2";
	if (argc >= 2) {
		filename = argv[1];
	}

    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    
    int sfont = fluid_synth_sfload(synth, filename, 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, NOTE_C, 80);

    for (int j=0; j <= 5 * MIRCO_SECOND / sample_duration_us(); j++)
    {
        for (int i = 0; i < synth->polyphony; i++) {
            fluid_voice_t *vt = synth->voice[i];
            if (_PLAYING(vt)) {
                // Test Loop sample 9176 - 10515 GMGSx.sf2
                unsigned int start = vt->sample->start;
                int phasei = fluid_phase_index(vt->phase);
                int diff = phasei-start;
                int diff_per = diff/(j+1);
                int phasef = fluid_phase_fract(vt->phase);
                printf("key:%d, diff:%d, diff_per:%d, phase_index:%d, phase_fract:%x\n", 
                    vt->key, diff, diff_per, phasei, phasef);
            }
        }

        fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
    }

    fluid_synth_noteoff(synth, 0, NOTE_C);

    free(buffer);

    delete_fluid_synth(synth);
    return 0;
}