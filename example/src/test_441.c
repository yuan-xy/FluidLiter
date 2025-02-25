#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"
#include "utils.c"
#include "fluid_sfont.h"

#define SAMPLE_RATE 44100

int main(int argc, char *argv[]) {
    char *fname = "pcm441.pcm";

    FILE *file = fopen(fname, "wb");

    fluid_synth_t *synth = NEW_FLUID_SYNTH();

    int sfid = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfid, 0, 0);

    int notes[] = {NN1, NN3, NN5, NN1 + 12};
    int frame = SAMPLE_RATE / 100;
    int16_t buffer[frame];
    for (int i = 0; i < 4; i++) {
        int note = notes[i % 4];
        fluid_synth_noteon(synth, 0, note, 127);
        for (int j = 0; j < 200; j++) {
            fluid_synth_write_s16_mono(synth, frame, buffer);
            fwrite(buffer, sizeof(int16_t), frame, file);
        }
        fluid_synth_noteoff(synth, 0, note);
    }

    fclose(file);
    delete_fluid_synth(synth);
    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 1 -i pcm441.pcm pcm441.wav");
    return 0;
}
