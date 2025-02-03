#ifndef _FLUID_CHORUS_H
#define _FLUID_CHORUS_H

#include "fluidsynth_priv.h"


typedef struct _fluid_chorus_t fluid_chorus_t;

/* enum describing each chorus parameter */
enum fluid_chorus_param
{
    FLUID_CHORUS_NR,        /**< number of delay line */
    FLUID_CHORUS_LEVEL,     /**< output level */
    FLUID_CHORUS_SPEED,     /**< lfo frequency */
    FLUID_CHORUS_DEPTH,     /**< modulation depth */
    FLUID_CHORUS_TYPE,      /**< type of waveform */
    FLUID_CHORUS_PARAM_LAST /* number of enum fluid_chorus_param */
};

/* return a bit flag from param: 2^param */
#define FLUID_CHORPARAM_TO_SETFLAG(param) (1 << param)

/** Flags for fluid_chorus_set() */
typedef enum
{
    FLUID_CHORUS_SET_NR    = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_NR),
    FLUID_CHORUS_SET_LEVEL = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_LEVEL),
    FLUID_CHORUS_SET_SPEED = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_SPEED),
    FLUID_CHORUS_SET_DEPTH = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_DEPTH),
    FLUID_CHORUS_SET_TYPE  = FLUID_CHORPARAM_TO_SETFLAG(FLUID_CHORUS_TYPE),

    /** Value for fluid_chorus_set() which sets all chorus parameters. */
    FLUID_CHORUS_SET_ALL   =   FLUID_CHORUS_SET_NR
                               | FLUID_CHORUS_SET_LEVEL
                               | FLUID_CHORUS_SET_SPEED
                               | FLUID_CHORUS_SET_DEPTH
                               | FLUID_CHORUS_SET_TYPE,
} fluid_chorus_set_t;






/*-------------------------------------------------------------------------------------
  Private
--------------------------------------------------------------------------------------*/
#define DEBUG_PRINT // allows message to be printed on the console.

#define MAX_CHORUS    99   /* number maximum of block */
#define MAX_LEVEL     10   /* max output level */
#define MIN_SPEED_HZ  0.1  /* min lfo frequency (Hz) */
#define MAX_SPEED_HZ  5    /* max lfo frequency (Hz) */

/* WIDTH [0..10] value define a stereo separation between left and right.
 When 0, the output is monophonic. When > 0 , the output is stereophonic.
 Actually WIDTH is fixed to maximum value. But in the future we could add a setting to
 allow a gradually stereo effect from minimum (monophonic) to maximum stereo effect.
*/
#define WIDTH 10

/* SCALE_WET_WIDTH is a compensation weight factor to get an output
   amplitude (wet) rather independent of the width setting.
    0: the output amplitude is fully dependent on the width setting.
   >0: the output amplitude is less dependent on the width setting.
   With a SCALE_WET_WIDTH of 0.2 the output amplitude is rather
   independent of width setting (see fluid_chorus_set()).
 */
#define SCALE_WET_WIDTH 0.2f
#define SCALE_WET 1.0f

#define MAX_SAMPLES 2048 /* delay length in sample (46.4 ms at sample rate: 44100Hz).*/
#define LOW_MOD_DEPTH 176             /* low mod_depth/2 in samples */
#define HIGH_MOD_DEPTH  MAX_SAMPLES/2 /* high mod_depth in sample */
#define RANGE_MOD_DEPTH (HIGH_MOD_DEPTH - LOW_MOD_DEPTH)

/* Important min max values for MOD_RATE */
/* mod rate define the rate at which the modulator is updated. Examples
   50: the modulator is updated every 50 samples (less cpu cycles expensive).
   1: the modulator is updated every sample (more cpu cycles expensive).
*/
/* MOD_RATE acceptable for max lfo speed (5Hz) and max modulation depth (46.6 ms) */
#define LOW_MOD_RATE 5  /* MOD_RATE acceptable for low modulation depth (8 ms) */
#define HIGH_MOD_RATE 4 /* MOD_RATE acceptable for max modulation depth (46.6 ms) */
                        /* and max lfo speed (5 Hz) */
#define RANGE_MOD_RATE (HIGH_MOD_RATE - LOW_MOD_RATE)

/* some chorus cpu_load measurement dependent of modulation rate: mod_rate
 (number of chorus blocks: 2)

 No stero unit:
 mod_rate | chorus cpu load(%) | one voice cpu load (%)
 ----------------------------------------------------
 50       | 0.204              |
 5        | 0.256              |  0.169
 1        | 0.417              |

 With stero unit:
 mod_rate | chorus cpu load(%) | one voice cpu load (%)
 ----------------------------------------------------
 50       | 0.220              |
 5        | 0.274              |  0.169
 1        | 0.465              |

*/

/*
 Number of samples to add to the desired length of the delay line. This
 allows to take account of rounding error interpolation when using large
 modulation depth.
 1 is sufficient for max modulation depth (46.6 ms) and max lfo speed (5 Hz).
*/
//#define INTERP_SAMPLES_NBR 0
#define INTERP_SAMPLES_NBR 1


/*-----------------------------------------------------------------------------
 Sinusoidal modulator
-----------------------------------------------------------------------------*/
/* modulator */
typedef struct
{
    // for sufficient precision members MUST be double! See https://github.com/FluidSynth/fluidsynth/issues/1331
    double   a1;           /* Coefficient: a1 = 2 * cos(w) */
    double   buffer1;      /* buffer1 */
    double   buffer2;      /* buffer2 */
    double   reset_buffer2;/* reset value of buffer2 */
} sinus_modulator;

/*-----------------------------------------------------------------------------
 Triangle modulator
-----------------------------------------------------------------------------*/
typedef struct
{
    fluid_real_t   freq;       /* Osc. Frequency (in Hertz) */
    fluid_real_t   val;         /* internal current value */
    fluid_real_t   inc;         /* increment value */
} triang_modulator;

/*-----------------------------------------------------------------------------
 modulator
-----------------------------------------------------------------------------*/
typedef struct
{
    /*-------------*/
    int line_out; /* current line out position for this modulator */
    /*-------------*/
    sinus_modulator sinus; /* sinus lfo */
    triang_modulator triang; /* triangle lfo */
    /*-------------------------*/
    /* first order All-Pass interpolator members */
    fluid_real_t  frac_pos_mod; /* fractional position part between samples */
    /* previous value used when interpolating using fractional */
    fluid_real_t  buffer;
} modulator;

/* Private data for SKEL file */
struct _fluid_chorus_t
{
    int type;
    fluid_real_t depth_ms;
    fluid_real_t level;
    fluid_real_t speed_Hz;
    int number_blocks;
    fluid_real_t sample_rate;

    /* width control: 0 monophonic, > 0 more stereophonic */
    fluid_real_t width;
    fluid_real_t wet1, wet2;

    fluid_real_t *line; /* buffer line */
    int   size;    /* effective internal size (in samples) */

    int line_in;  /* line in position */

    /* center output position members */
    fluid_real_t  center_pos_mod; /* center output position modulated by modulator */
    int          mod_depth;   /* modulation depth (in samples) */

    /* variable rate control of center output position */
    int index_rate;  /* index rate to know when to update center_pos_mod */
    int mod_rate;    /* rate at which center_pos_mod is updated */

    /* modulator member */
    modulator mod[MAX_CHORUS]; /* sinus/triangle modulator */
};





fluid_chorus_t *new_fluid_chorus(fluid_real_t sample_rate);
void delete_fluid_chorus(fluid_chorus_t *chorus);
void fluid_chorus_reset(fluid_chorus_t *chorus);

void fluid_chorus_set(fluid_chorus_t *chorus, int set, int nr, fluid_real_t level,
                      fluid_real_t speed, fluid_real_t depth_ms, int type);
void
fluid_chorus_samplerate_change(fluid_chorus_t *chorus, fluid_real_t sample_rate);

void fluid_chorus_processmix(fluid_chorus_t *chorus, const fluid_real_t *in,
                             fluid_real_t *left_out, fluid_real_t *right_out);
void fluid_chorus_processreplace(fluid_chorus_t *chorus, const fluid_real_t *in,
                                 fluid_real_t *left_out, fluid_real_t *right_out);



#endif /* _FLUID_CHORUS_H */
