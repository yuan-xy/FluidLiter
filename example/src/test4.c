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
#define NUM_FRAMES 17640                    // SAMPLE_RATE*DURATION
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
    if (argc < 2) {
      printf("Usage: %s <soundfont>\n", argv[0]);
      return 1;
    }
    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 1);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, C, 80);

    for (int j=0; j <= 5 * MIRCO_SECOND / sample_duration_us(); j++)
    {
        char filename[32];
        snprintf(filename, 31, "test1_%d.pcm", j);
        printf("write file %s.\n", filename);
        FILE *file = fopen(filename, "wb");
        fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);
        fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);
        fclose(file);
    }

    fluid_synth_noteoff(synth, 0, C);

    free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}