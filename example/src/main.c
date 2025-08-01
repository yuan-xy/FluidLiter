#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "fluidliter.h"
#include "misc.h"

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(int16_t) //s16
#define DURATION 2 //second
#define NUM_FRAMES SAMPLE_RATE*DURATION
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

 
    fluid_synth_t* synth = NEW_FLUID_SYNTH(.gain=0.4);

    int sfont = fluid_synth_sfload(synth, argv[1], 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES+NUM_SAMPLES/10);

    FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;

    clock_t start, end;
    double cpu_time_used;
    start = clock(); 
    fluid_synth_noteon(synth, 0, 60, 50);
    fluid_synth_noteon(synth, 0, 67, 80);
    fluid_synth_noteon(synth, 0, 76, 100);
    fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("code exec time: %f seconds\n", cpu_time_used);

    for(int i=0; i< NUM_FRAMES; i++){
      //printf("i%d: %d\n", i, buffer[i]);
      buffer[i] *= 16;
    }

    fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);
    printf("calculateVolumeDB(buffer, NUM_SAMPLES): %f\n", calculateVolumeDB(buffer, NUM_SAMPLES));
    assert(float_eq(calculateVolumeDB(buffer, NUM_SAMPLES), -14.874127));


    fluid_synth_noteoff(synth, 0, 60);
    fluid_synth_noteoff(synth, 0, 67);
    fluid_synth_noteoff(synth, 0, 76);

    int16_t *buffer2 = buffer+NUM_SAMPLES;
    fluid_synth_write_s16(synth, NUM_FRAMES/10, buffer2, 0, NUM_CHANNELS, buffer2, 1, NUM_CHANNELS);
    fwrite(buffer2, SAMPLE_SIZE, NUM_SAMPLES/10, file);

    fclose(file);


    int compare(const void *a, const void *b) {
        return (*(int16_t *)a - *(int16_t *)b);
    };
    qsort(buffer, NUM_SAMPLES, SAMPLE_SIZE, compare);
    printf("最小值: %d, 最大值: %d\n", buffer[0], buffer[NUM_SAMPLES-1]);
    assert(buffer[0] >= -32766);
    assert(buffer[NUM_SAMPLES-1] < 32766);

    free(buffer);

    delete_fluid_synth(synth);
}
