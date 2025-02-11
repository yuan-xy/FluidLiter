#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include "fluidlite.h"
#include "fluid_synth.h"
#include "utils.c"
#include "fluid_sfont.h"
#include "misc.h"
#include "fluid_chorus.h"
#include "fluid_voice.h"

#define SAMPLE_RATE 44100


int main(){
    set_log_level(FLUID_DBG);
    FILE* file = fopen("effects.pcm", "wb");

    fluid_synth_t *synth;
    synth = NEW_FLUID_SYNTH(.with_reverb=false, .gain = 1.5);
    int sfid = fluid_synth_sfload(synth, "/mnt/c/SF2/GeneralUser-GS.sf2", 1);
    if(sfid == FLUID_FAILED) return 0;

//  Each of the sound elements in an EMU8000 consists of the following:

//  Oscillator
//       An oscillator is the source of an audio signal.
//  Low Pass Filter
//       The low pass filter is responsible for modifying the timbres of an
//       instrument. The low pass filter's filter cutoff values can be varied
//       from 100 Hz to 8000 Hz. By changing the values of the filter cutoff, a
//       myriad of analogue sounding filter sweeps can be achieved. An example
//       of a GM instrument that makes use of filter sweep is instrument number
//       87, Lead 7 (fifths).

    fluid_synth_program_select(synth, 0, sfid, 0, 86);

    int frame = 5*SAMPLE_RATE;
    int samples = frame * 2;

    assert(synth->voice[0]->volenv_count == 0);
    assert(synth->voice[0]->modenv_count == 0);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVDELAY);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVDELAY);

    fluid_synth_noteon(synth, 0, 70, 127);
    assert(synth->voice[0]->gen[Gen_VolEnvDecay].val == 3986.f);
    assert(synth->voice[0]->gen[Gen_VolEnvRelease].val == 0.f);

    assert(synth->voice[0]->volenv_count == 0);
    assert(synth->voice[0]->modenv_count == 0);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVDELAY);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVDELAY);

    int16_t buffer[samples];
    fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
    fwrite(buffer, sizeof(int16_t), samples, file);

    assert(synth->voice[0]->volenv_count == 2755);
    assert(synth->voice[0]->modenv_count == 2755);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVSUSTAIN);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVSUSTAIN);

    fluid_synth_noteoff(synth, 0, 70);

    assert(synth->voice[0]->volenv_count == 0);
    assert(synth->voice[0]->modenv_count == 0);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVRELEASE);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVRELEASE);

    fclose(file);
    delete_fluid_synth(synth);
    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i effects.pcm effects.wav");

    return 0;
}