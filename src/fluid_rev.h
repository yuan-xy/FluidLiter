#ifndef _FLUID_REV_H
#define _FLUID_REV_H

#include "fluidsynth_priv.h"

typedef struct _fluid_revmodel_t fluid_revmodel_t;

/* enum describing each reverb parameter */
enum fluid_reverb_param
{
    FLUID_REVERB_ROOMSIZE,  /**< reverb time */
    FLUID_REVERB_DAMP,      /**< high frequency damping */
    FLUID_REVERB_WIDTH,     /**< stereo width */
    FLUID_REVERB_LEVEL,     /**< output level */
    FLUID_REVERB_PARAM_LAST /* number of enum fluid_reverb_param */
};

/* return a bit flag from param: 2^param */
#define FLUID_REVPARAM_TO_SETFLAG(param) (1 << param)

/** Flags for fluid_revmodel_set() */
typedef enum
{
    FLUID_REVMODEL_SET_ROOMSIZE       = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_ROOMSIZE),
    FLUID_REVMODEL_SET_DAMPING        = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_DAMP),
    FLUID_REVMODEL_SET_WIDTH          = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_WIDTH),
    FLUID_REVMODEL_SET_LEVEL          = FLUID_REVPARAM_TO_SETFLAG(FLUID_REVERB_LEVEL),

    /** Value for fluid_revmodel_set() which sets all reverb parameters. */
    FLUID_REVMODEL_SET_ALL            =   FLUID_REVMODEL_SET_LEVEL
                                          | FLUID_REVMODEL_SET_WIDTH
                                          | FLUID_REVMODEL_SET_DAMPING
                                          | FLUID_REVMODEL_SET_ROOMSIZE,
} fluid_revmodel_set_t;

/*
 * reverb preset
 */
typedef struct _fluid_revmodel_presets_t
{
    const char *name;
    fluid_real_t roomsize;
    fluid_real_t damp;
    fluid_real_t width;
    fluid_real_t level;
} fluid_revmodel_presets_t;


/*----------------------------------------------------------------------------
                        Configuration macros at compiler time.

 3 macros are usable at compiler time:
  - NBR_DELAYs: number of delay lines. 8 (default) or 12.
  - ROOMSIZE_RESPONSE_LINEAR: allows to choose an alternate response for
    roomsize parameter.
  - DENORMALISING enable denormalising handling.
-----------------------------------------------------------------------------*/
//#define INFOS_PRINT /* allows message to be printed on the console. */

/* Number of delay lines (must be only 8 or 12)
  8 is the default.
 12 produces a better quality but is +50% cpu expensive.
*/
#define NBR_DELAYS        8 /* default*/

/* response curve of parameter roomsize  */
/*
    The default response is the same as Freeverb:
    - roomsize (0 to 1) controls concave reverb time (0.7 to 12.5 s).

    when ROOMSIZE_RESPONSE_LINEAR is defined, the response is:
    - roomsize (0 to 1) controls reverb time linearly  (0.7 to 12.5 s).
*/
//#define ROOMSIZE_RESPONSE_LINEAR

/* DENORMALISING enable denormalising handling */
#define DENORMALISING

#ifdef DENORMALISING
#define DC_OFFSET 1e-8f
#else
#define DC_OFFSET  0.0f
#endif



/*-----------------------------------------------------------------------------
 Delay absorbent low pass filter
-----------------------------------------------------------------------------*/
typedef struct
{
    fluid_real_t buffer;
    fluid_real_t b0, a1;         /* filter coefficients */
} fdn_delay_lpf;


/*-----------------------------------------------------------------------------
 Delay line :
 The delay line is composed of the line plus an absorbent low pass filter
 to get frequency dependent reverb time.
-----------------------------------------------------------------------------*/
typedef struct
{
    fluid_real_t *line; /* buffer line */
    int   size;         /* effective internal size (in samples) */
    /*-------------*/
    int line_in;  /* line in position */
    int line_out; /* line out position */
    /*-------------*/
    fdn_delay_lpf damping; /* damping low pass filter */
} delay_line;


/*-----------------------------------------------------------------------------
 Sinusoidal modulator
-----------------------------------------------------------------------------*/
/* modulator are integrated in modulated delay line */
typedef struct
{
    fluid_real_t   a1;          /* Coefficient: a1 = 2 * cos(w) */
    fluid_real_t   buffer1;     /* buffer1 */
    fluid_real_t   buffer2;     /* buffer2 */
    fluid_real_t   reset_buffer2;/* reset value of buffer2 */
} sinus_modulator_rev;


/*-----------------------------------------------------------------------------
 Modulated delay line. The line is composed of:
 - the delay line with its damping low pass filter.
 - the sinusoidal modulator.
 - center output position modulated by the modulator.
 - variable rate control of center output position.
 - first order All-Pass interpolator.
-----------------------------------------------------------------------------*/
typedef struct
{
    /* delay line with damping low pass filter member */
    delay_line dl; /* delayed line */
    /*---------------------------*/
    /* Sinusoidal modulator member */
    sinus_modulator_rev mod; /* sinus modulator */
    /*-------------------------*/
    /* center output position members */
    fluid_real_t  center_pos_mod; /* center output position modulated by modulator */
    int          mod_depth;   /* modulation depth (in samples) */
    /*-------------------------*/
    /* variable rate control of center output position */
    int index_rate;  /* index rate to know when to update center_pos_mod */
    int mod_rate;    /* rate at which center_pos_mod is updated */
    /*-------------------------*/
    /* first order All-Pass interpolator members */
    fluid_real_t  frac_pos_mod; /* fractional position part between samples) */
    /* previous value used when interpolating using fractional */
    fluid_real_t  buffer;
} mod_delay_line;

/*-----------------------------------------------------------------------------
 Late structure
-----------------------------------------------------------------------------*/
struct _fluid_late
{
    fluid_real_t samplerate;       /* sample rate */
    fluid_real_t sample_rate_max;  /* sample rate maximum */
    /*----- High pass tone corrector -------------------------------------*/
    fluid_real_t tone_buffer;
    fluid_real_t b1, b2;
    /*----- Modulated delay lines lines ----------------------------------*/
    mod_delay_line mod_delay_lines[NBR_DELAYS];
    /*-----------------------------------------------------------------------*/
    /* Output coefficients for separate Left and right stereo outputs */
    fluid_real_t out_left_gain[NBR_DELAYS]; /* Left delay lines' output gains */
    fluid_real_t out_right_gain[NBR_DELAYS];/* Right delay lines' output gains*/
};

typedef struct _fluid_late   fluid_late;
/*-----------------------------------------------------------------------------
 fluidsynth reverb structure
-----------------------------------------------------------------------------*/
struct _fluid_revmodel_t
{
    /* reverb parameters */
    fluid_real_t roomsize; /* acting on reverb time */
    fluid_real_t damp; /* acting on frequency dependent reverb time */
    fluid_real_t level, wet1, wet2; /* output level */
    fluid_real_t width; /* width stereo separation */

    /* fdn reverberation structure */
    fluid_late  late;
};


/*
 * reverb
 */
fluid_revmodel_t *
new_fluid_revmodel(fluid_real_t sample_rate_max, fluid_real_t sample_rate);

void delete_fluid_revmodel(fluid_revmodel_t *rev);

void fluid_revmodel_processmix(fluid_revmodel_t *rev, const fluid_real_t *in,
                               fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_processreplace(fluid_revmodel_t *rev, const fluid_real_t *in,
                                   fluid_real_t *left_out, fluid_real_t *right_out);

void fluid_revmodel_reset(fluid_revmodel_t *rev);

void fluid_revmodel_set(fluid_revmodel_t *rev, int set, fluid_real_t roomsize,
                        fluid_real_t damping, fluid_real_t width, fluid_real_t level);

int fluid_revmodel_samplerate_change(fluid_revmodel_t *rev, fluid_real_t sample_rate);

#endif /* _FLUID_REV_H */
