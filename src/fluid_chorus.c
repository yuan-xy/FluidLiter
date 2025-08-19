/*
  based on a chorus implementation made by Juergen Mueller And Sundry Contributors in 1998

  CHANGES

  - Adapted for fluidsynth, Peter Hanappe, March 2002

  - Variable delay line implementation using bandlimited
    interpolation, code reorganization: Markus Nentwig May 2002

  - Complete rewrite using lfo computed on the fly, first order all-pass
    interpolator and adding stereo unit: Jean-Jacques Ceresa, Jul 2019
 */


/*
 * 	Chorus effect.
 *
 * Flow diagram scheme for n delays ( 1 <= n <= MAX_CHORUS ):
 *
 *                                                       ________
 *                  direct signal (not implemented) >-->|        |
 *                 _________                            |        |
 * mono           |         |                           |        |
 * input ---+---->| delay 1 |-------------------------->| Stereo |---> right
 *          |     |_________|                           |        |     output
 *          |        /|\                                | Unit   |
 *          :         |                                 |        |
 *          : +-----------------+                       |(width) |
 *          : | Delay control 1 |<-+                    |        |
 *          : +-----------------+  |                    |        |---> left
 *          |      _________       |                    |        |     output
 *          |     |         |      |                    |        |
 *          +---->| delay n |-------------------------->|        |
 *                |_________|      |                    |        |
 *                   /|\           |                    |________|
 *                    |            |  +--------------+      /|\
 *            +-----------------+  |  |mod depth (ms)|       |
 *            | Delay control n |<-*--|lfo speed (Hz)|     gain-out
 *            +-----------------+     +--------------+
 *
 *
 * The delay i is controlled by a sine or triangle modulation i ( 1 <= i <= n).
 *
 * The chorus unit process a monophonic input signal and produces stereo output
 * controlled by WIDTH macro.
 * Actually WIDTH is fixed to maximum value. But in the future, we could add a
 * setting (e.g "synth.chorus.width") allowing the user to get a gradually stereo
 * effect from minimum (monophonic) to maximum stereo effect.
 *
 * Delays lines are implemented using only one line for all chorus blocks.
 * Each chorus block has it own lfo (sinus/triangle). Each lfo are out of phase
 * to produce uncorrelated signal at the output of the delay line (this simulates
 * the presence of individual line for each block). Each lfo modulates the length
 * of the line using a depth modulation value and lfo frequency value common to
 * all lfos.
 *
 * LFO modulators are computed on the fly, instead of using lfo lookup table.
 * The advantages are:
 * - Avoiding a lost of 608272 memory bytes when lfo speed is low (0.3Hz).
 * - Allows to diminish the lfo speed lower limit to 0.1Hz instead of 0.3Hz.
 *   A speed of 0.1 is interesting for chorus. Using a lookuptable for 0.1Hz
 *   would require too much memory (1824816 bytes).
 * - Interpolation make use of first order all-pass interpolator instead of
 *   bandlimited interpolation.
 * - Although lfo modulator is computed on the fly, cpu load is lower than
 *   using lfo lookup table with bandlimited interpolator.
 */

#include "fluid_chorus.h"

#ifdef EMPTY_CHORUS
fluid_chorus_t *new_fluid_chorus(fluid_real_t sample_rate){return EMPTY_CHORUS_STUB;}
void delete_fluid_chorus(fluid_chorus_t *chorus){}
void fluid_chorus_reset(fluid_chorus_t *chorus){}

void fluid_chorus_set(fluid_chorus_t *chorus, int set, int nr, fluid_real_t level,
                      fluid_real_t speed, fluid_real_t depth_ms, int type){}
void
fluid_chorus_samplerate_change(fluid_chorus_t *chorus, fluid_real_t sample_rate){}

void fluid_chorus_processmix(fluid_chorus_t *chorus, const fluid_real_t *in,
                             fluid_real_t *left_out, fluid_real_t *right_out){}
void fluid_chorus_processreplace(fluid_chorus_t *chorus, const fluid_real_t *in,
                                 fluid_real_t *left_out, fluid_real_t *right_out){}
#else

/*-----------------------------------------------------------------------------
 Sets the frequency of sinus oscillator.

 For sufficient precision use double precision in set_sinus_frequency() computation !.
 Never use: fluid_real_t , cosf(), sinf(), FLUID_COS(), FLUID_SIN(), FLUID_M_PI.
 See https://github.com/FluidSynth/fluidsynth/issues/1331

 @param mod pointer on modulator structure.
 @param freq frequency of the oscillator in Hz.
 @param sample_rate sample rate on audio output in Hz.
 @param phase initial phase of the oscillator in degree (0 to 360).
-----------------------------------------------------------------------------*/
static void set_sinus_frequency(sinus_modulator *mod,
                                float freq, float sample_rate, float phase)
{
    double w = (2.0 * M_PI) * freq / sample_rate;  /* step phase between each sinus wave sample (in radian) */
    double a; /* initial phase at which the sinus wave must begin (in radian) */

    // DO NOT use potentially single precision cosf or FLUID_COS here! See https://github.com/FluidSynth/fluidsynth/issues/1331
    mod->a1 = 2 * cos(w);

    a = (2.0 * M_PI / 360.0) * phase;

    mod->buffer2 = sin(a - w); /* y(n-1) = sin(-initial angle) */
    mod->buffer1 = sin(a); /* y(n) = sin(initial phase) */
    mod->reset_buffer2 = sin((M_PI / 2.0) - w); /* reset value for PI/2 */
}

/*-----------------------------------------------------------------------------
 Gets current value of sinus modulator:
   y(n) = a1 . y(n-1)  -  y(n-2)
   out = a1 . buffer1  -  buffer2

 @param mod pointer on modulator structure.
 @return current value of the modulator sine wave.
-----------------------------------------------------------------------------*/
static FLUID_INLINE double get_mod_sinus(sinus_modulator *mod)
{
    double out;
    out = mod->a1 * mod->buffer1 - mod->buffer2;
    mod->buffer2 = mod->buffer1;

    if(out >= 1.0) /* reset in case of instability near PI/2 */
    {
        out = 1.0; /* forces output to the right value */
        mod->buffer2 = mod->reset_buffer2;
    }

    if(out <= -1.0) /* reset in case of instability near -PI/2 */
    {
        out = -1.0; /* forces output to the right value */
        mod->buffer2 = - mod->reset_buffer2;
    }

    mod->buffer1 = out;
    return  out;
}

/*-----------------------------------------------------------------------------
 Set the frequency of triangular oscillator
 The frequency is converted in a slope value.
 The initial value is set according to frac_phase which is a position
 in the period relative to the beginning of the period.
 For example: 0 is the beginning of the period, 1/4 is at 1/4 of the period
 relative to the beginning.

 @param mod pointer on modulator structure.
 @param freq frequency of the oscillator in Hz.
 @param sample_rate sample rate on audio output in Hz.
 @param frac_phase initial phase (see comment above).
-----------------------------------------------------------------------------*/
static void set_triangle_frequency(triang_modulator *mod, float freq,
                                   float sample_rate, float frac_phase)
{
    fluid_real_t ns_period; /* period in numbers of sample */

    if(freq <= 0.0)
    {
        freq = 0.5f;
    }

    mod->freq = freq;

    ns_period = sample_rate / freq;

    /* the slope of a triangular osc (0 up to +1 down to -1 up to 0....) is equivalent
    to the slope of a saw osc (0 -> +4) */
    mod->inc  = 4 / ns_period; /* positive slope */

    /* The initial value and the sign of the slope depend of initial phase:
      initial value = = (ns_period * frac_phase) * slope
    */
    mod->val =  ns_period * frac_phase * mod->inc;

    if(1.0 <= mod->val && mod->val < 3.0)
    {
        mod->val = 2.0 - mod->val; /*  1.0 down to -1.0 */
        mod->inc = -mod->inc; /* negative slope */
    }
    else if(3.0 <= mod->val)
    {
        mod->val = mod->val - 4.0; /*  -1.0 up to +1.0. */
    }

    /* else val < 1.0 */
}

/*-----------------------------------------------------------------------------
   Get current value of triangular oscillator
       y(n) = y(n-1) + dy

 @param mod pointer on triang_modulator structure.
 @return current value.
-----------------------------------------------------------------------------*/
static FLUID_INLINE fluid_real_t get_mod_triang(triang_modulator *mod)
{
    mod->val = mod->val + mod->inc ;

    if(mod->val >= 1.0)
    {
        mod->inc = -mod->inc;
        return 1.0;
    }

    if(mod->val <= -1.0)
    {
        mod->inc = -mod->inc;
        return -1.0;
    }

    return  mod->val;
}
/*-----------------------------------------------------------------------------
 Reads the sample value out of the modulated delay line.

 @param chorus pointer on chorus unit.
 @param mod pointer on modulator structure.
 @return current value.
-----------------------------------------------------------------------------*/
static FLUID_INLINE fluid_real_t get_mod_delay(fluid_chorus_t *chorus,
                                               modulator *mod)
{
    fluid_real_t out_index;  /* new modulated index position */
    int int_out_index; /* integer part of out_index */
    fluid_real_t out; /* value to return */

    /* Checks if the modulator must be updated (every mod_rate samples). */
    /* Important: center_pos_mod must be used immediately for the
       first sample. So, mdl->index_rate must be initialized
       to mdl->mod_rate (new_mod_delay_line())  */

    if(chorus->index_rate >= chorus->mod_rate)
    {
        /* out_index = center position (center_pos_mod) + sinus waweform */
        if(chorus->type == FLUID_CHORUS_MOD_SINE)
        {
            out_index = chorus->center_pos_mod +
                        get_mod_sinus(&mod->sinus) * chorus->mod_depth;
        }
        else
        {
            out_index = chorus->center_pos_mod +
                        get_mod_triang(&mod->triang) * chorus->mod_depth;
        }

        /* extracts integer part in int_out_index */
        if(out_index >= 0.0f)
        {
            int_out_index = (int)out_index; /* current integer part */

            /* forces read index (line_out)  with integer modulation value  */
            /* Boundary check and circular motion as needed */
            if((mod->line_out = int_out_index) >= chorus->size)
            {
                mod->line_out -= chorus->size;
            }
        }
        else /* negative */
        {
            int_out_index = (int)(out_index - 1); /* previous integer part */
            /* forces read index (line_out) with integer modulation value  */
            /* circular motion as needed */
            mod->line_out   = int_out_index + chorus->size;
        }

        /* extracts fractionnal part. (it will be used when interpolating
          between line_out and line_out +1) and memorize it.
          Memorizing is necessary for modulation rate above 1 */
        mod->frac_pos_mod = out_index - int_out_index;
    }

    /*  First order all-pass interpolation ----------------------------------*/
    /* https://ccrma.stanford.edu/~jos/pasp/First_Order_Allpass_Interpolation.html */
    /*  begins interpolation: read current sample */
    out = chorus->line[mod->line_out];

    /* updates line_out to the next sample.
       Boundary check and circular motion as needed */
    if(++mod->line_out >= chorus->size)
    {
        mod->line_out -= chorus->size;
    }

    /* Fractional interpolation between next sample (at next position) and
       previous output added to current sample.
    */
    out += mod->frac_pos_mod * (chorus->line[mod->line_out] - mod->buffer);
    mod->buffer = out; /* memorizes current output */
    return out;
}

/*-----------------------------------------------------------------------------
 Push a sample val into the delay line

 @param dl delay line to push value into.
 @param val the value to push into dl.
-----------------------------------------------------------------------------*/
#define push_in_delay_line(dl, val) \
{\
    dl->line[dl->line_in] = val;\
    /* Incrementation and circular motion if necessary */\
    if(++dl->line_in >= dl->size) dl->line_in -= dl->size;\
}\

/*-----------------------------------------------------------------------------
 Initialize : mod_rate, center_pos_mod,  and index rate

 center_pos_mod is initialized so that the delay between center_pos_mod and
 line_in is: mod_depth + INTERP_SAMPLES_NBR.

 @param chorus pointer on chorus unit.
-----------------------------------------------------------------------------*/
static void set_center_position(fluid_chorus_t *chorus)
{
    int center;

    /* Sets the modulation rate. This rate defines how often
     the center position (center_pos_mod ) is modulated .
     The value is expressed in samples. The default value is 1 that means that
     center_pos_mod is updated at every sample.
     For example with a value of 2, the center position position will be
     updated only one time every 2 samples only.
    */
    chorus->mod_rate = LOW_MOD_RATE; /* default modulation rate */

    /* compensate mod rate for high modulation depth */
    if(chorus->mod_depth > LOW_MOD_DEPTH)
    {
        int delta_mod_depth = (chorus->mod_depth - LOW_MOD_DEPTH);
        chorus->mod_rate += (delta_mod_depth * RANGE_MOD_RATE) / RANGE_MOD_DEPTH;
    }

    /* Initializes the modulated center position (center_pos_mod) so that:
        - the delay between center_pos_mod and line_in is:
          mod_depth + INTERP_SAMPLES_NBR.
    */
    center = chorus->line_in - (INTERP_SAMPLES_NBR + chorus->mod_depth);

    if(center < 0)
    {
        center += chorus->size;
    }

    chorus->center_pos_mod = (fluid_real_t)center;

    /* index rate to control when to update center_pos_mod */
    /* Important: must be set to get center_pos_mod immediately used for the
       reading of first sample (see get_mod_delay()) */
    chorus->index_rate = chorus->mod_rate;
}

/*-----------------------------------------------------------------------------
 Update internal parameters dependent of sample rate.
 - mod_depth.
 - mod_rate, center_pos_mod, and index rate.
 - modulators frequency.

 @param chorus, pointer on chorus unit.
-----------------------------------------------------------------------------*/
static void update_parameters_from_sample_rate(fluid_chorus_t *chorus)
{
    int i;

    /* initialize modulation depth (peak to peak) (in samples) */
    /* convert modulation depth in ms to sample number */
    chorus->mod_depth = (int)(chorus->depth_ms  / 1000.0
                              * chorus->sample_rate);

    /* the delay line is fixed. So we reduce mod_depth (if necessary) */
    if(chorus->mod_depth > MAX_SAMPLES)
    {
        FLUID_LOG(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).",
                  MAX_SAMPLES);
        chorus->mod_depth = MAX_SAMPLES;
        /* set depth_ms to maximum to avoid spamming console with above warning */
        chorus->depth_ms = (chorus->mod_depth * 1000) / chorus->sample_rate;
    }

    chorus->mod_depth /= 2; /* amplitude is peak to peek / 2 */
#ifdef DEBUG_PRINT
    FLUID_LOG(FLUID_DBG, "depth_ms:%f, depth_samples/2:%d\n", chorus->depth_ms, chorus->mod_depth);
#endif

    /* Initializes the modulated center position:
       mod_rate, center_pos_mod, and index rate.
    */
    set_center_position(chorus); /* must be called before set_xxxx_frequency() */
#ifdef DEBUG_PRINT
    FLUID_LOG(FLUID_DBG, "mod_rate:%d\n", chorus->mod_rate);
#endif

    /* initialize modulator frequency */
    for(i = 0; i < chorus->number_blocks; i++)
    {
        set_sinus_frequency(&chorus->mod[i].sinus,
                            chorus->speed_Hz * chorus->mod_rate,
                            chorus->sample_rate,
                            /* phase offset between modulators waveform */
                            (float)((360.0f / (float) chorus->number_blocks) * i));

        set_triangle_frequency(&chorus->mod[i].triang,
                               chorus->speed_Hz * chorus->mod_rate,
                               chorus->sample_rate,
                               /* phase offset between modulators waveform */
                               (float)i / chorus->number_blocks);
    }
}

/*-----------------------------------------------------------------------------
 Modulated delay line initialization.

 Sets the length line ( alloc delay samples).
 Remark: the function sets the internal size according to the length delay_length.
 The size is augmented by INTERP_SAMPLES_NBR to take account of interpolation.

 @param chorus, pointer on chorus unit.
 @param delay_length the length of the delay line in samples.
 @return FLUID_OK if success , FLUID_FAILED if memory error.

 Return FLUID_OK if success, FLUID_FAILED if memory error.
-----------------------------------------------------------------------------*/
static int new_mod_delay_line(fluid_chorus_t *chorus, int delay_length)
{
    /*-----------------------------------------------------------------------*/
    /* checks parameter */
    if(delay_length < 1)
    {
        return FLUID_FAILED;
    }

    chorus->mod_depth = 0;
    /*-----------------------------------------------------------------------
     allocates delay_line and initialize members: - line, size, line_in...
    */
    /* total size of the line:  size = INTERP_SAMPLES_NBR + delay_length */
    chorus->size = delay_length + INTERP_SAMPLES_NBR;
    chorus->line = FLUID_ARRAY(fluid_real_t, chorus->size);

    if(! chorus->line)
    {
        return FLUID_FAILED;
    }

    /* clears the buffer:
     - delay line
     - interpolator member: buffer, frac_pos_mod
    */
    fluid_chorus_reset(chorus);

    /* Initializes line_in to the start of the buffer */
    chorus->line_in = 0;
    /*------------------------------------------------------------------------
     Initializes modulation members:
     - modulation rate (the speed at which center_pos_mod is modulated: mod_rate
     - modulated center position: center_pos_mod
     - index rate to know when to update center_pos_mod:index_rate
     -------------------------------------------------------------------------*/
    /* Initializes the modulated center position:
       mod_rate, center_pos_mod,  and index rate
    */
    set_center_position(chorus);

    return FLUID_OK;
}

/*-----------------------------------------------------------------------------
  API
------------------------------------------------------------------------------*/
/**
 * Create the chorus unit. Once created the chorus have no parameters set, so
 * fluid_chorus_set() must be called at least one time after calling
 * new_fluid_chorus().
 *
 * @param sample_rate, audio sample rate in Hz.
 * @return pointer on chorus unit.
 */
fluid_chorus_t *
new_fluid_chorus(fluid_real_t sample_rate)
{
    fluid_chorus_t *chorus;

    chorus = FLUID_NEW(fluid_chorus_t);

    if(chorus == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "chorus: Out of memory");
        return NULL;
    }

    FLUID_MEMSET(chorus, 0, sizeof(fluid_chorus_t));

    chorus->sample_rate = sample_rate;

    if(new_mod_delay_line(chorus, MAX_SAMPLES) == FLUID_FAILED)
    {
        delete_fluid_chorus(chorus);
        return NULL;
    }

    return chorus;
}

/**
 * Delete the chorus unit.
 * @param chorus pointer on chorus unit returned by new_fluid_chorus().
 */
void
delete_fluid_chorus(fluid_chorus_t *chorus)
{
    fluid_return_if_fail(chorus != NULL);

    FLUID_FREE(chorus->line);
    FLUID_FREE(chorus);
}

/**
 * Clear the internal delay line and associate filter.
 * @param chorus pointer on chorus unit returned by new_fluid_chorus().
 */
void
fluid_chorus_reset(fluid_chorus_t *chorus)
{
    int i;
    unsigned int u;

    /* reset delay line */
    for(i = 0; i < chorus->size; i++)
    {
        chorus->line[i] = 0;
    }

    /* reset modulators's allpass filter */
    for(u = 0; u < FLUID_N_ELEMENTS(chorus->mod); u++)
    {
        /* initializes 1st order All-Pass interpolator members */
        chorus->mod[u].buffer = 0;       /* previous delay sample value */
        chorus->mod[u].frac_pos_mod = 0; /* fractional position (between consecutives sample) */
    }
}

/**
 * Set one or more chorus parameters.
 *
 * @param chorus Chorus instance.
 * @param set Flags indicating which chorus parameters to set (#fluid_chorus_set_t).
 * @param nr Chorus voice count (0-99, CPU time consumption proportional to
 *   this value).
 * @param level Chorus level (0.0-10.0).
 * @param speed Chorus speed in Hz (0.1-5.0).
 * @param depth_ms Chorus depth (max value depends on synth sample rate,
 *   0.0-21.0 is safe for sample rate values up to 96KHz).
 * @param type Chorus waveform type (#fluid_chorus_mod).
 */
void
fluid_chorus_set(fluid_chorus_t *chorus, int set, int nr, fluid_real_t level,
                 fluid_real_t speed, fluid_real_t depth_ms, int type)
{
    if(set & FLUID_CHORUS_SET_NR) /* number of block */
    {
        chorus->number_blocks = nr;
    }

    if(set & FLUID_CHORUS_SET_LEVEL) /* output level */
    {
        chorus->level = level;
    }

    if(set & FLUID_CHORUS_SET_SPEED) /* lfo frequency (in Hz) */
    {
        chorus->speed_Hz = speed;
    }

    if(set & FLUID_CHORUS_SET_DEPTH) /* modulation depth (in ms) */
    {
        chorus->depth_ms = depth_ms;
    }

    if(set & FLUID_CHORUS_SET_TYPE) /* lfo shape (sinus, triangle) */
    {
        chorus->type = type;
    }

    /* check min , max parameters */
    if(chorus->number_blocks < 0)
    {
        FLUID_LOG(FLUID_WARN, "chorus: number blocks must be >=0! Setting value to 0.");
        chorus->number_blocks = 0;
    }
    else if(chorus->number_blocks > MAX_CHORUS)
    {
        FLUID_LOG(FLUID_WARN, "chorus: number blocks larger than max. allowed! Setting value to %d.",
                  MAX_CHORUS);
        chorus->number_blocks = MAX_CHORUS;
    }

    if(chorus->speed_Hz < MIN_SPEED_HZ)
    {
        FLUID_LOG(FLUID_WARN, "chorus: speed is too low (min %f)! Setting value to min.",
                  (double) MIN_SPEED_HZ);
        chorus->speed_Hz = MIN_SPEED_HZ;
    }
    else if(chorus->speed_Hz > MAX_SPEED_HZ)
    {
        FLUID_LOG(FLUID_WARN, "chorus: speed must be below %f Hz! Setting value to max.",
                  (double) MAX_SPEED_HZ);
        chorus->speed_Hz = MAX_SPEED_HZ;
    }

    if(chorus->depth_ms < 0.0)
    {
        FLUID_LOG(FLUID_WARN, "chorus: depth must be positive! Setting value to 0.");
        chorus->depth_ms = 0.0;
    }

    if(chorus->level < 0.0)
    {
        FLUID_LOG(FLUID_WARN, "chorus: level must be positive! Setting value to 0.");
        chorus->level = 0.0;
    }
    else if(chorus->level > MAX_LEVEL)
    {
        FLUID_LOG(FLUID_WARN, "chorus: level must be < 10. A reasonable level is << 1! "
                  "Setting it to 0.1.");
        chorus->level = 0.1;
    }

    /* update parameters dependent of sample rate */
    update_parameters_from_sample_rate(chorus);

#ifdef DEBUG_PRINT
    FLUID_LOG(FLUID_DBG, "lfo type:%d\n", chorus->type);
    FLUID_LOG(FLUID_DBG, "speed_Hz:%f\n", chorus->speed_Hz);
#endif

    /* Initialize the lfo waveform */
    if((chorus->type != FLUID_CHORUS_MOD_SINE) &&
            (chorus->type != FLUID_CHORUS_MOD_TRIANGLE))
    {
        FLUID_LOG(FLUID_WARN, "chorus: Unknown modulation type. Using sinewave.");
        chorus->type = FLUID_CHORUS_MOD_SINE;
    }

#ifdef DEBUG_PRINT

    if(chorus->type == FLUID_CHORUS_MOD_SINE)
    {
        FLUID_LOG(FLUID_DBG, "lfo: sinus\n");
    }
    else
    {
        FLUID_LOG(FLUID_DBG, "lfo: triangle\n");
    }

    FLUID_LOG(FLUID_DBG, "nr:%d\n", chorus->number_blocks);
#endif

    /* Recalculate internal values after parameters change */

    /*
     Note:
     Actually WIDTH is fixed to maximum value. But in the future we could add a setting
     "synth.chorus.width" to allow a gradually stereo effect from minimum (monophonic) to
     maximum stereo effect.
     If this setting will be added, remove the following instruction.
    */
    chorus->width = WIDTH;
    {
        /* The stereo amplitude equation (wet1 and wet2 below) have a
         tendency to produce high amplitude with high width values ( 1 < width < 10).
         This results in an unwanted noisy output clipped by the audio card.
         To avoid this dependency, we divide by (1 + chorus->width * SCALE_WET_WIDTH)
         Actually, with a SCALE_WET_WIDTH of 0.2, (regardless of level setting),
         the output amplitude (wet) seems rather independent of width setting */

        fluid_real_t wet = chorus->level * SCALE_WET ;

        /* wet1 and wet2 are used by the stereo effect controlled by the width setting
        for producing a stereo ouptput from a monophonic chorus signal.
        Please see the note above about a side effect tendency */

        if(chorus->number_blocks > 1)
        {
            wet = wet  / (1.0f + chorus->width * SCALE_WET_WIDTH);
            chorus->wet1 = wet * (chorus->width / 2.0f + 0.5f);
            chorus->wet2 = wet * ((1.0f - chorus->width) / 2.0f);
#ifdef DEBUG_PRINT
            FLUID_LOG(FLUID_DBG, "width:%f\n", chorus->width);

            if(chorus->width > 0)
            {
                FLUID_LOG(FLUID_DBG, "nr > 1, width > 0 => out stereo\n");
            }
            else
            {
                FLUID_LOG(FLUID_DBG, "nr > 1, width:0 =>out mono\n");
            }

#endif
        }
        else
        {
            /* only one chorus block */
            if(chorus->width == 0.0)
            {
                /* wet1 and wet2 should make stereo output monomophic */
                chorus->wet1 = chorus->wet2 = wet;
            }
            else
            {
                /* for width > 0, wet1 and wet2 should make stereo output stereo
                   with only one block. This will only possible by inverting
                   the unique signal on each left and right output.
                   Note however that with only one block, it isn't possible to
                   have a graduate width effect */
                chorus->wet1  = wet;
                chorus->wet2  = -wet; /* inversion */
            }

#ifdef DEBUG_PRINT
            FLUID_LOG(FLUID_DBG, "width:%f\n", chorus->width);

            if(chorus->width != 0)
            {
                FLUID_LOG(FLUID_DBG, "one block, width > 0 => out stereo\n");
            }
            else
            {
                FLUID_LOG(FLUID_DBG, "one block,  width:0 => out mono\n");
            }

#endif
        }
    }
}

/*
* Applies a sample rate change on the chorus.
* Note that while the chorus is used by calling any fluid_chorus_processXXX()
* function, calling fluid_chorus_samplerate_change() isn't multi task safe.
* To deal properly with this issue follow the steps:
* 1) Stop chorus processing (i.e disable calling to any fluid_chorus_processXXX().
*    chorus functions.
* 2) Change sample rate by calling fluid_chorus_samplerate_change().
* 3) Restart chorus processing (i.e enabling calling any fluid_chorus_processXXX()
*    chorus functions.
*
* Another solution is to substitute step (2):
* 2.1) delete the chorus by calling delete_fluid_chorus().
* 2.2) create the chorus by calling new_fluid_chorus().
*
* @param chorus pointer on the chorus.
* @param sample_rate new sample rate value.
*/
void
fluid_chorus_samplerate_change(fluid_chorus_t *chorus, fluid_real_t sample_rate)
{
    chorus->sample_rate = sample_rate;

    /* update parameters dependent of sample rate */
    update_parameters_from_sample_rate(chorus);
}

/**
 * Process chorus by mixing the result in output buffer.
 * @param chorus pointer on chorus unit returned by new_fluid_chorus().
 * @param in, pointer on monophonic input buffer of FLUID_BUFSIZE samples.
 * @param left_out, right_out, pointers on stereo output buffers of
 *  FLUID_BUFSIZE samples.
 */
void fluid_chorus_processmix(fluid_chorus_t *chorus, const fluid_real_t *in,
                             fluid_real_t *left_out, fluid_real_t *right_out)
{
    int sample_index;
    int i;
    fluid_real_t d_out[2];               /* output stereo Left and Right  */

    /* foreach sample, process output sample then input sample */
    for(sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++)
    {
        fluid_real_t out=0.0f; /* block output */

        d_out[0] = d_out[1] = 0.0f; /* clear stereo unit input */

#if 0
        /* Debug: Listen to the chorus signal only */
        left_out[sample_index] = 0;
        right_out[sample_index] = 0;
#endif

        ++chorus->index_rate; /* modulator rate */

        /* foreach chorus block, process output sample */
        for(i = 0; i < chorus->number_blocks; i++)
        {
            /* get sample from the output of modulated delay line */
            out = get_mod_delay(chorus, &chorus->mod[i]);

            /* accumulate out into stereo unit input */
            d_out[i & 1] += out;
        }

        /* update modulator index rate and output center position */
        if(chorus->index_rate >= chorus->mod_rate)
        {
            chorus->index_rate = 0; /* clear modulator index rate */

            /* updates center position (center_pos_mod) to the next position
               specified by modulation rate */
            if((chorus->center_pos_mod += chorus->mod_rate) >= chorus->size)
            {
                chorus->center_pos_mod -= chorus->size;
            }
        }

        /* Adjust stereo input level in case of number_blocks odd:
           In those case, d_out[1] level is lower than d_out[0], so we need to
           add out value to d_out[1] to have d_out[0] and d_out[1] balanced.
        */
        if((i & 1) && i > 2)  // i = 3,5,7...
        {
            d_out[1] +=  out ;
        }

        /* Write the current input sample into the circular buffer.
         * Note that 'in' may be aliased with 'left_out'. Hence this must be done
         * before "processing stereo unit" (below). This ensures input buffer
         * not being overwritten by stereo unit output.
         */
        push_in_delay_line(chorus, in[sample_index]);

        /* process stereo unit */
        /* Add the chorus stereo unit d_out to left and right output */
        left_out[sample_index]  += d_out[0] * chorus->wet1  + d_out[1] * chorus->wet2;
        right_out[sample_index] += d_out[1] * chorus->wet1  + d_out[0] * chorus->wet2;
    }
}

/**
 * Process chorus by putting the result in output buffer (no mixing).
 * @param chorus pointer on chorus unit returned by new_fluid_chorus().
 * @param in, pointer on monophonic input buffer of FLUID_BUFSIZE samples.
 * @param left_out, right_out, pointers on stereo output buffers of
 *  FLUID_BUFSIZE samples.
 */
/* Duplication of code ... (replaces sample data instead of mixing) */
void fluid_chorus_processreplace(fluid_chorus_t *chorus, const fluid_real_t *in,
                                 fluid_real_t *left_out, fluid_real_t *right_out)
{
    int sample_index;
    int i;
    fluid_real_t d_out[2];               /* output stereo Left and Right  */

    /* foreach sample, process output sample then input sample */
    for(sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++)
    {
        fluid_real_t out=0.0f; /* block output */

        d_out[0] = d_out[1] = 0.0f; /* clear stereo unit input */

#if 0
        /* Debug: Listen to the chorus signal only */
        left_out[sample_index] = 0;
        right_out[sample_index] = 0;
#endif

        ++chorus->index_rate; /* modulator rate */

        /* foreach chorus block, process output sample */
        for(i = 0; i < chorus->number_blocks; i++)
        {
            /* get sample from the output of modulated delay line */
            out = get_mod_delay(chorus, &chorus->mod[i]);

            /* accumulate out into stereo unit input */
            d_out[i & 1] += out;
        }

        /* update modulator index rate and output center position */
        if(chorus->index_rate >= chorus->mod_rate)
        {
            chorus->index_rate = 0; /* clear modulator index rate */

            /* updates center position (center_pos_mod) to the next position
               specified by modulation rate */
            if((chorus->center_pos_mod += chorus->mod_rate) >= chorus->size)
            {
                chorus->center_pos_mod -= chorus->size;
            }
        }

        /* Adjust stereo input level in case of number_blocks odd:
           In those case, d_out[1] level is lower than d_out[0], so we need to
           add out value to d_out[1] to have d_out[0] and d_out[1] balanced.
        */
        if((i & 1) && i > 2)  // i = 3,5,7...
        {
            d_out[1] +=  out ;
        }

        /* Write the current input sample into the circular buffer.
         * Note that 'in' may be aliased with 'left_out'. Hence this must be done
         * before "processing stereo unit" (below). This ensures input buffer
         * not being overwritten by stereo unit output.
         */
        push_in_delay_line(chorus, in[sample_index]);

        /* process stereo unit */
        /* store the chorus stereo unit d_out to left and right output */
        left_out[sample_index]  = d_out[0] * chorus->wet1  + d_out[1] * chorus->wet2;
        right_out[sample_index] = d_out[1] * chorus->wet1  + d_out[0] * chorus->wet2;
    }
}

#endif //EMPTY_CHORUS