#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "fluidlite.h"

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(int16_t) //s16
#define DURATION 2 //second
#define NUM_FRAMES SAMPLE_RATE*DURATION
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    fluid_settings_t* settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "yes"); //在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 5); 
    fluid_settings_setint(settings, "synth.midi-channels", 1); 
    fluid_synth_t* synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);
    FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;

    fluid_synth_noteon(synth, 0, 60, 50);
    fluid_synth_noteon(synth, 0, 67, 80);
    fluid_synth_noteon(synth, 0, 76, 100);
    fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);

    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);

    fluid_synth_noteoff(synth, 0, 60);
    fluid_synth_noteoff(synth, 0, 67);
    fluid_synth_noteoff(synth, 0, 76);
    fluid_synth_write_s16_mono(synth, NUM_FRAMES/10, buffer);
    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES/10, file);

    fclose(file);
    free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}
