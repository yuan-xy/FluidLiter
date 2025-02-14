#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"
#include "misc.h"
#include <float.h> // 用于定义 FLT_MIN
#define __USE_GNU
#include <fenv.h>
#include <signal.h>

void handle_fpe(int sig) {
    printf("Floating point exception caught: %d!\n", sig);
    assert(sig==SIGFPE);
    exit(0);
}

void handle_fpe1(int sig) {
    printf("Shoundn't happen: %d!\n", sig);
    exit(1);
}


#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 44100                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)




int main(int argc, char *argv[])
{
#ifndef __linux__
    return 0;
#else

    signal(SIGFPE, handle_fpe1);

    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);

    fluid_synth_t *synth = NEW_FLUID_SYNTH();
    set_log_level(FLUID_WARN);
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    fluid_synth_noteon(synth, 0, NOTE_C, 100);
    fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 1, buffer, 1, 2);
    assert(fetestexcept(FE_INEXACT));

    signal(SIGFPE, handle_fpe); 
    feenableexcept(FE_INEXACT);
    fluid_synth_write_s16(synth, NUM_FRAMES, buffer, 0, 1, buffer, 1, 2);
    assert(false); //不应该执行到这里

    free(buffer);
    delete_fluid_synth(synth);
    return 0;
#endif

}

