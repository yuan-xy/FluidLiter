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

//  http://archive.gamedev.net/archive/reference/articles/article445.html
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
    assert(synth->voice[0]->gen[Gen_VolEnvDecay].val == 3986.f);//TODO: why this value? global preset volEnvDecay=10.0
    assert(synth->voice[0]->gen[Gen_VolEnvRelease].val == 0.f);

    assert(synth->voice[0]->volenv_count == 0);
    assert(synth->voice[0]->modenv_count == 0);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVDELAY);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVDELAY);

    int16_t buffer[samples];
    fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
    fwrite(buffer, sizeof(int16_t), samples, file);

    if(sizeof(fluid_real_t) == 4){
        assert(synth->voice[0]->volenv_count == 2755);
    }else{
        assert(synth->voice[0]->volenv_count == 2756);
    }
    
    assert(synth->voice[0]->modenv_count == 2755);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVSUSTAIN);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVSUSTAIN);

    fluid_synth_noteoff(synth, 0, 70);

    assert(synth->voice[0]->volenv_count == 0);
    assert(synth->voice[0]->modenv_count == 0);
    assert(synth->voice[0]->volenv_section == FLUID_VOICE_ENVRELEASE);
    assert(synth->voice[0]->modenv_section == FLUID_VOICE_ENVRELEASE);

// Amplifier
//     The amplifier determines the loudness of an audio signal.
// LFO1
//     An LFO, or Low Frequency Oscillator, is normally used to periodically
//     modulate, that is, change a sound parameter, whether it be volume
//     (amplitude modulation), pitch (frequency modulation) or filter cutoff
//     (filter modulation). It operates at sub-audio frequency from 0.042 Hz
//     to 10.71 Hz. The LFO1 in the EMU8000 modulates the pitch, volume and
//     filter cutoff simultaneously.
// LFO2
//     The LFO2 is similar to the LFO1, except that it modulates the pitch of
//     the audio signal only.
// Resonance
//     A filter alone would be like an equalizer, making a bright audio
//     signal duller, but the addition of resonance greatly increases the
//     creative potential of a filter. Increasing the resonance of a filter
//     makes it emphasize signals at the cutoff frequency, giving the audio
//     signal a subtle wah-wah, that is, imagine a siren sound going from
//     bright to dull to bright again periodically.
// LFO1 to Volume (Tremolo)
//     The LFO1's output is routed to the amplifier, with the depth of
//     oscillation determined by LFO1 to Volume. LFO1 to Volume produces
//     tremolo, which is a periodic fluctuation of volume. Lets say you are
//     listening to a piece of music on your home stereo system. When you
//     rapidly increase and decrease the playback volume, you are creating
//     tremolo effect, and the speed in which you increases and decreases the
//     volume is the tremolo rate (which corresponds to the speed at which
//     the LFO is oscillating). An example of a GM instrument that makes use
//     of LFO1 to Volume is instrument number 45, Tremolo Strings.

    fluid_synth_program_select(synth, 0, sfid, 0, 44);
    fluid_synth_noteon(synth, 0, 70, 127);
    fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
    fwrite(buffer, sizeof(int16_t), samples, file);
    fluid_synth_noteoff(synth, 0, 70);

// LFO1 to Filter Cutoff (Wah-Wah)
//     The LFO1's output is routed to the filter, with the depth of
//     oscillation determined by LFO1 to Filter. LFO1 to Filter produces a
//     periodic fluctuation in the filter cutoff frequency, producing an
//     effect very similar to that of a wah-wah guitar (see resonance for a
//     description of wah-wah) An example of a GM instrument that makes use
//     of LFO1 to Filter Cutoff is instrument number 19, Rock Organ.

    fluid_synth_program_select(synth, 0, sfid, 0, 18);
    fluid_synth_noteon(synth, 0, 70, 127);
    fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
    fwrite(buffer, sizeof(int16_t), samples, file);
    fluid_synth_noteoff(synth, 0, 70);

// LFO1 to Pitch (Vibrato)
//     The LFO1's output is routed to the oscillator, with the depth of
//     oscillation determined by LFO1 to Pitch. LFO1 to Pitch produces a
//     periodic fluctuation in the pitch of the oscillator, producing a
//     vibrato effect. An example of a GM instrument that makes use of LFO1
//     to Pitch is instrument number 57, Trumpet.

    fluid_synth_program_select(synth, 0, sfid, 0, 56);
    fluid_synth_noteon(synth, 0, 70, 127);
    fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
    fwrite(buffer, sizeof(int16_t), samples, file);
    fluid_synth_noteoff(synth, 0, 70);

// LFO2 to Pitch (Vibrato)
//     The LFO1 in the EMU8000 can simultaneously modulate pitch, volume and
//     filter. LFO2, on the other hand, modulates only the pitch, with the
//     depth of modulation determined by LFO2 to Pitch. LFO2 to Pitch
//     produces a periodic fluctuation in the pitch of the oscillator,
//     producing a vibrato effect. When this is coupled with LFO1 to Pitch, a
//     complex vibrato effect can be achieved.
// Volume Envelope
//     The character of a musical instrument is largely determined by its
//     volume envelope, the way in which the level of the sound changes with
//     time. For example, percussive sounds usually start suddenly and then
//     die away, whereas a bowed sound might take quite some time to start
//     and then sustain at a more or less fixed level.

//     A six-stage envelope makes up the volume envelope of the EMU8000. The
//     six stages are delay, attack, hold, decay, sustain and release. The
//     stages can be described as follows:
//     Delay
//          The time between when a key is played and when the attack phase
//          begins
//     Attack
//          The time it takes to go from zero to the peak (full) level.
//     Hold
//          The time the envelope will stay at the peak level before starting
//          the decay phase.
//     Decay
//          The time it takes the envelope to go from the peak level to the
//          sustain level.
//     Sustain
//          The level at which the envelope remains as long as a key is held
//          down.
//     Release
//          The time it takes the envelope to fall to the zero level after
//          the key is released.

//     Using these six parameters can yield very realistic reproduction of
//          the volume envelope characteristics of many musical instruments.
//     Pitch and Filter Envelope
//          The pitch and filter envelope is similar to the volume envelope in
//          that it has the same envelope stages. The difference between them is
//          that whereas the volume envelope contours the volume of the instrument
//          over time, the pitch and filter envelope contours the pitch and filter
//          values of the instrument over time. The pitch envelope is particularly
//          useful in putting the finishing touches in simulating a natural
//          instrument. For example, some wind instruments tend to go slightly
//          sharp when they are first blown, and this characteristic can be
//          simulated by setting up a pitch envelope with a fairly fast attack and
//          decay. The filter envelope, on the other hand, is useful in creating
//          synthetic sci-fi sound textures. An example of a GM instrument that
//          makes use of the filter envelope is instrument number 86, Pad 8
//          (Sweep).

    fluid_synth_program_select(synth, 0, sfid, 0, 95);
    fluid_synth_noteon(synth, 0, 70, 127);
    fluid_synth_write_s16(synth, frame, buffer, 0, 2, buffer, 1, 2);
    fwrite(buffer, sizeof(int16_t), samples, file);
    fluid_synth_noteoff(synth, 0, 70);

    fclose(file);
    delete_fluid_synth(synth);
    system("ffmpeg -hide_banner -y -f s16le -ar 44100 -ac 2 -i effects.pcm effects.wav");

    return 0;
}