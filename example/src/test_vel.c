#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidlite.h"
#include "fluid_synth.h"
#include "misc.h"

#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 44100                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)


int main(int argc, char *argv[])
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "synth.verbose", "no"); // 在新版本中"synth.verbose"是int型
    fluid_settings_setint(settings, "synth.polyphony", 3);
    fluid_settings_setint(settings, "synth.midi-channels", 1);
    fluid_settings_setstr(settings, "synth.reverb.active", "no");

    fluid_synth_t *synth = new_fluid_synth(settings);
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

	for(int i=10; i<=120; i+=10){
        char *fname = "velocity.pcm";
        FILE* file = fopen(fname, "wb");
        fluid_synth_noteon(synth, 0, C, i);
        fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 1, buffer, 1, 2);
        fluid_synth_noteoff(synth, 0, C);
        fwrite(buffer, sizeof(uint16_t), NUM_SAMPLES, file);
        if(true){
            printf("Velocity %d:\t %.1fDB\n", i, calculateVolumeDB(buffer, NUM_SAMPLES));
            calculateAndPrintVolume(buffer, NUM_SAMPLES);
        }else{
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i velocity.pcm velocity%d.wav", i);
            system(cmd);
        }
	}

    free(buffer);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 0;
}

