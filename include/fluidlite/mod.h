#ifndef _FLUIDSYNTH_MOD_H
#define _FLUIDSYNTH_MOD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Modulator-related definitions */

/* Maximum number of modulators in a voice */
#define FLUID_NUM_MOD 9 // only support default modulators

/*
 *  fluid_mod_t
 */
struct _fluid_mod_t {
    unsigned char dest;
    unsigned char src1;
    unsigned char flags1;
    unsigned char src2;
    unsigned char flags2;
    fluid_real_t amount;
    /* The 'next' field allows to link modulators into a list.  It is
     * not used in fluid_voice.c, there each voice allocates memory for a
     * fixed number of modulators.  Since there may be a huge number of
     * different zones, this is more efficient.
     */
    fluid_mod_t *next;
};

/* Flags telling the polarity of a modulator.  Compare with SF2.01
   section 8.2. Note: The numbers of the bits are different!  (for
   example: in the flags of a SF modulator, the polarity bit is bit
   nr. 9) */
enum fluid_mod_flags {
    FLUID_MOD_POSITIVE = 0,
    FLUID_MOD_NEGATIVE = 1,
    FLUID_MOD_UNIPOLAR = 0,
    FLUID_MOD_BIPOLAR = 2,
    FLUID_MOD_LINEAR = 0,
    FLUID_MOD_CONCAVE = 4,
    FLUID_MOD_CONVEX = 8,
    FLUID_MOD_SWITCH = 12,
    FLUID_MOD_GC = 0,
    FLUID_MOD_CC = 16
};

/* Flags telling the source of a modulator.  This corresponds to
 * SF2.01 section 8.2.1 */
enum fluid_mod_src {
    FLUID_MOD_NONE = 0,
    FLUID_MOD_VELOCITY = 2,
    FLUID_MOD_KEY = 3,
    FLUID_MOD_KEYPRESSURE = 10,
    FLUID_MOD_CHANNELPRESSURE = 13,
    FLUID_MOD_PITCHWHEEL = 14,
    FLUID_MOD_PITCHWHEELSENS = 16
};

/* Allocates memory for a new modulator */
fluid_mod_t *fluid_mod_new(void);

/* Frees the modulator */
void fluid_mod_delete(fluid_mod_t *mod);

void fluid_mod_set_source1(fluid_mod_t *mod, int src, int flags);
void fluid_mod_set_source2(fluid_mod_t *mod, int src, int flags);
void fluid_mod_set_dest(fluid_mod_t *mod, int dst);
void fluid_mod_set_amount(fluid_mod_t *mod, fluid_real_t amount);

int fluid_mod_get_source1(fluid_mod_t *mod);
int fluid_mod_get_flags1(fluid_mod_t *mod);
int fluid_mod_get_source2(fluid_mod_t *mod);
int fluid_mod_get_flags2(fluid_mod_t *mod);
int fluid_mod_get_dest(fluid_mod_t *mod);
fluid_real_t fluid_mod_get_amount(fluid_mod_t *mod);

/* Determines, if two modulators are 'identical' (all parameters
   except the amount match) */
int fluid_mod_test_identity(fluid_mod_t *mod1,
                                           fluid_mod_t *mod2);

#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_MOD_H */
