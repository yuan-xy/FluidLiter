#ifndef _FLUIDSYNTH_VOICE_H
#define _FLUIDSYNTH_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

/** Update all the synthesis parameters, which depend on generator gen.
    This is only necessary after changing a generator of an already operating
   voice. Most applications will not need this function.*/

void fluid_voice_update_param(fluid_voice_t *voice, int gen);

/* for fluid_voice_add_mod */
enum fluid_voice_add_mod {
    FLUID_VOICE_OVERWRITE,
    FLUID_VOICE_ADD,
    FLUID_VOICE_DEFAULT
};

/* Add a modulator to a voice (SF2.1 only). */
void fluid_voice_add_mod(fluid_voice_t *voice, fluid_mod_t *mod,
                                        int mode);

/** Set the value of a generator */
void fluid_voice_gen_set(fluid_voice_t *voice, int gen,
                                        float val);

/** Get the value of a generator */
float fluid_voice_gen_get(fluid_voice_t *voice, int gen);

/** Modify the value of a generator by val */
void fluid_voice_gen_incr(fluid_voice_t *voice, int gen,
                                         float val);

/** Return the unique ID of the noteon-event. A sound font loader
 *  may store the voice processes it has created for * real-time
 *  control during the operation of a voice (for example: parameter
 *  changes in sound font editor). The synth uses a pool of
 *  voices, which are 'recycled' and never deallocated.
 *
 * Before modifying an existing voice, check
 * - that its state is still 'playing'
 * - that the ID is still the same
 * Otherwise the voice has finished playing.
 */
unsigned int fluid_voice_get_id(fluid_voice_t *voice);

int fluid_voice_is_playing(fluid_voice_t *voice);

/** If the peak volume during the loop is known, then the voice can
 * be released earlier during the release phase. Otherwise, the
 * voice will operate (inaudibly), until the envelope is at the
 * nominal turnoff point. In many cases the loop volume is many dB
 * below the maximum volume.  For example, the loop volume for a
 * typical acoustic piano is 20 dB below max.  Taking that into
 * account in the turn-off algorithm we can save 20 dB / 100 dB =>
 * 1/5 of the total release time.
 * So it's a good idea to call fluid_voice_optimize_sample
 * on each sample once.
 */

int fluid_voice_optimize_sample(fluid_sample_t *s);

#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_VOICE_H */
