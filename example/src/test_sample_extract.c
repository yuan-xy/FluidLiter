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


void test_sample_index(fluid_sfont_t *sfont) {
    fluid_list_t *list;
    fluid_sample_t *sample;
    int index = 0;
    for (list = sfont->sample; list; list = fluid_list_next(list)) {
        sample = (fluid_sample_t *)fluid_list_get(list);

        assert(sample->idx_in_sfont == index);
        index++;
    }
}


int main(int argc, char *argv[])
{
    char *fname = "test_sample.pcm";
    FILE* file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    int id = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, id, 0, 0);
    fluid_sfont_t *sfont = (fluid_sfont_t *)synth->sfont->data;
    short *sampledata = sfont->sampledata;
    printf("samplesize:%d\n", sfont->samplesize);
    assert(sfont->sample_count == 10);
    assert(sfont->samplesize == 185544);
    test_sample_index(sfont);

    // fwrite(sampledata, sizeof(short), sfont->samplesize/sizeof(short), file);

    short out[185544];
    for(int j=0; j< 185544/2; j++){
        out[2*j] = sampledata[j];
        out[2*j+1] = (sampledata[j] + sampledata[j+1])/2;
    }
    fwrite(out, sizeof(short), 185544, file);


    // int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    // fluid_synth_noteon(synth, 0, NOTE_C, 127);
	// fluid_synth_write_s16_mono(synth, NUM_FRAMES, (uint16_t *)buffer);
    // fluid_synth_noteoff(synth, 0, NOTE_C);



    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i test_sample.pcm test_sample.wav >nul 2>&1");


    // free(buffer);

    delete_fluid_synth(synth);

    return 0;
}

