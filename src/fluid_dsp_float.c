#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_voice.h"
#include "fluid_phase.h"

/* Purpose:
 *
 * Interpolates audio data (obtains values between the samples of the original
 * waveform data).
 *
 * Variables loaded from the voice structure (assigned in fluid_voice_write()):
 * - dsp_data: Pointer to the original waveform data
 * - dsp_phase: The position in the original waveform data.
 *              This has an integer and a fractional part (between samples).
 * - dsp_phase_incr: For each output sample, the position in the original
 *              waveform advances by dsp_phase_incr. This also has an integer
 *              part and a fractional part.
 *              If a sample is played at root pitch (no pitch change),
 *              dsp_phase_incr is integer=1 and fractional=0.
 * - dsp_amp: The current amplitude envelope value.
 * - dsp_amp_incr: The changing rate of the amplitude envelope.
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_i: Index through the output buffer
 * - dsp_buf: Output buffer of floating point values (FLUID_BUFSIZE in length)
 */



/* Interpolation (find a value between two samples of the original waveform) */

/* Linear interpolation table (2 coefficients centered on 1st) */
static fluid_real_t interp_coeff_linear[FLUID_INTERP_MAX][2];

/* 4th order (cubic) interpolation table (4 coefficients centered on 2nd) */
static fluid_real_t interp_coeff[FLUID_INTERP_MAX][4];


/* Initializes interpolation tables */
void fluid_dsp_float_config (void)
{
  int i, i2;
  double x, v;
  double i_shifted;

  /* Initialize the coefficients for the interpolation. The math comes
   * from a mail, posted by Olli Niemitalo to the music-dsp mailing
   * list (I found it in the music-dsp archives
   * http://www.smartelectronix.com/musicdsp/).  */

  for (i = 0; i < FLUID_INTERP_MAX; i++)
  {
    x = (double) i / (double) FLUID_INTERP_MAX;

    interp_coeff[i][0] = (fluid_real_t)(x * (-0.5 + x * (1 - 0.5 * x)));
    interp_coeff[i][1] = (fluid_real_t)(1.0 + x * x * (1.5 * x - 2.5));
    interp_coeff[i][2] = (fluid_real_t)(x * (0.5 + x * (2.0 - 1.5 * x)));
    interp_coeff[i][3] = (fluid_real_t)(0.5 * x * x * (x - 1.0));

    interp_coeff_linear[i][0] = (fluid_real_t)(1.0 - x);
    interp_coeff_linear[i][1] = (fluid_real_t)x;
  }
}


/* No interpolation. Just take the sample, which is closest to
  * the playback pointer.  Questionable quality, but very
  * efficient. */
int
fluid_dsp_float_interpolate_none (fluid_voice_t *voice)
{
  fluid_phase_t dsp_phase = voice->phase;
  fluid_phase_t dsp_phase_incr;
  short int *dsp_data = voice->sample->data;
  fluid_real_t *dsp_buf = voice->dsp_buf;
  fluid_real_t dsp_amp = voice->amp;
  fluid_real_t dsp_amp_incr = voice->amp_incr;
  unsigned int dsp_i = 0;
  unsigned int dsp_phase_index;
  unsigned int end_index;
  int looping;

  /* Convert playback "speed" floating point value to phase index/fract */
  fluid_phase_set_float (dsp_phase_incr, voice->phase_incr);

  /* voice is currently looping? */
  looping = _SAMPLEMODE (voice) == FLUID_LOOP_DURING_RELEASE
    || (_SAMPLEMODE (voice) == FLUID_LOOP_UNTIL_RELEASE
	&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

  end_index = looping ? voice->loopend - 1 : voice->end;

  while (1)
  {
    dsp_phase_index = fluid_phase_index_round (dsp_phase);	/* round to nearest point */

    /* interpolate sequence of sample points */
    for ( ; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
    {
      dsp_buf[dsp_i] = dsp_amp * dsp_data[dsp_phase_index];

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index_round (dsp_phase);	/* round to nearest point */
      dsp_amp += dsp_amp_incr;
    }

    /* break out if not looping (buffer may not be full) */
    if (!looping) break;

    /* go back to loop start */
    if (dsp_phase_index > end_index)
    {
      fluid_phase_sub_int (dsp_phase, voice->loopend - voice->loopstart);
      voice->has_looped = 1;
    }

    /* break out if filled buffer */
    if (dsp_i >= FLUID_BUFSIZE) break;
  }

  voice->phase = dsp_phase;
  voice->amp = dsp_amp;

  return (dsp_i);
}

/* Straight line interpolation.
 * Returns number of samples processed (usually FLUID_BUFSIZE but could be
 * smaller if end of sample occurs).
 */
int
fluid_dsp_float_interpolate_linear (fluid_voice_t *voice)
{
  fluid_phase_t dsp_phase = voice->phase;
  fluid_phase_t dsp_phase_incr;
  short int *dsp_data = voice->sample->data;
  fluid_real_t *dsp_buf = voice->dsp_buf;
  fluid_real_t dsp_amp = voice->amp;
  fluid_real_t dsp_amp_incr = voice->amp_incr;
  unsigned int dsp_i = 0;
  unsigned int dsp_phase_index;
  unsigned int end_index;
  short int point;
  fluid_real_t *coeffs;
  int looping;

  /* Convert playback "speed" floating point value to phase index/fract */
  fluid_phase_set_float (dsp_phase_incr, voice->phase_incr);

  /* voice is currently looping? */
  looping = _SAMPLEMODE (voice) == FLUID_LOOP_DURING_RELEASE
    || (_SAMPLEMODE (voice) == FLUID_LOOP_UNTIL_RELEASE
	&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

  /* last index before 2nd interpolation point must be specially handled */
  end_index = (looping ? voice->loopend - 1 : voice->end) - 1;

  /* 2nd interpolation point to use at end of loop or sample */
  if (looping) point = dsp_data[voice->loopstart];	/* loop start */
  else point = dsp_data[voice->end];			/* duplicate end for samples no longer looping */

  while (1)
  {
    dsp_phase_index = fluid_phase_index (dsp_phase);

    /* interpolate the sequence of sample points */
    for ( ; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
    {
      coeffs = interp_coeff_linear[fluid_phase_fract_to_tablerow (dsp_phase)];
      dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index]
				  + coeffs[1] * dsp_data[dsp_phase_index+1]);

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index (dsp_phase);
      dsp_amp += dsp_amp_incr;
    }

    /* break out if buffer filled */
    if (dsp_i >= FLUID_BUFSIZE) break;

    end_index++;	/* we're now interpolating the last point */

    /* interpolate within last point */
    for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
    {
      coeffs = interp_coeff_linear[fluid_phase_fract_to_tablerow (dsp_phase)];
      dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index]
				  + coeffs[1] * point);

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index (dsp_phase);
      dsp_amp += dsp_amp_incr;	/* increment amplitude */
    }

    if (!looping) break;	/* break out if not looping (end of sample) */

    /* go back to loop start (if past */
    if (dsp_phase_index > end_index)
    {
      fluid_phase_sub_int (dsp_phase, voice->loopend - voice->loopstart);
      voice->has_looped = 1;
    }

    /* break out if filled buffer */
    if (dsp_i >= FLUID_BUFSIZE) break;

    end_index--;	/* set end back to second to last sample point */
  }

  voice->phase = dsp_phase;
  voice->amp = dsp_amp;

  return (dsp_i);
}

/* 4th order (cubic) interpolation.
 * Returns number of samples processed (usually FLUID_BUFSIZE but could be
 * smaller if end of sample occurs).
 */
int
fluid_dsp_float_interpolate_4th_order (fluid_voice_t *voice)
{
  fluid_phase_t dsp_phase = voice->phase;
  fluid_phase_t dsp_phase_incr;
  short int *dsp_data = voice->sample->data;
  fluid_real_t *dsp_buf = voice->dsp_buf;
  fluid_real_t dsp_amp = voice->amp;
  fluid_real_t dsp_amp_incr = voice->amp_incr;
  unsigned int dsp_i = 0;
  unsigned int dsp_phase_index;
  unsigned int start_index, end_index;
  short int start_point, end_point1, end_point2;
  fluid_real_t *coeffs;
  int looping;

  /* Convert playback "speed" floating point value to phase index/fract */
  fluid_phase_set_float (dsp_phase_incr, voice->phase_incr);

  /* voice is currently looping? */
  looping = _SAMPLEMODE (voice) == FLUID_LOOP_DURING_RELEASE
    || (_SAMPLEMODE (voice) == FLUID_LOOP_UNTIL_RELEASE
	&& voice->volenv_section < FLUID_VOICE_ENVRELEASE);

  /* last index before 4th interpolation point must be specially handled */
  end_index = (looping ? voice->loopend - 1 : voice->end) - 2;

  if (voice->has_looped)	/* set start_index and start point if looped or not */
  {
    start_index = voice->loopstart;
    start_point = dsp_data[voice->loopend - 1];	/* last point in loop (wrap around) */
  }
  else
  {
    start_index = voice->start;
    start_point = dsp_data[voice->start];	/* just duplicate the point */
  }

  /* get points off the end (loop start if looping, duplicate point if end) */
  if (looping)
  {
    end_point1 = dsp_data[voice->loopstart];
    end_point2 = dsp_data[voice->loopstart + 1];
  }
  else
  {
    end_point1 = dsp_data[voice->end];
    end_point2 = end_point1;
  }

  while (1)
  {
    dsp_phase_index = fluid_phase_index (dsp_phase);

    /* interpolate first sample point (start or loop start) if needed */
    for ( ; dsp_phase_index == start_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
    {
      coeffs = interp_coeff[fluid_phase_fract_to_tablerow (dsp_phase)];
      dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * start_point
				  + coeffs[1] * dsp_data[dsp_phase_index]
				  + coeffs[2] * dsp_data[dsp_phase_index+1]
				  + coeffs[3] * dsp_data[dsp_phase_index+2]);

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index (dsp_phase);
      dsp_amp += dsp_amp_incr;
    }

    /* interpolate the sequence of sample points */
    for ( ; dsp_i < FLUID_BUFSIZE && dsp_phase_index <= end_index; dsp_i++)
    {
      coeffs = interp_coeff[fluid_phase_fract_to_tablerow (dsp_phase)];
      dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index-1]
				  + coeffs[1] * dsp_data[dsp_phase_index]
				  + coeffs[2] * dsp_data[dsp_phase_index+1]
				  + coeffs[3] * dsp_data[dsp_phase_index+2]);

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index (dsp_phase);
      dsp_amp += dsp_amp_incr;
    }

    /* break out if buffer filled */
    if (dsp_i >= FLUID_BUFSIZE) break;

    end_index++;	/* we're now interpolating the 2nd to last point */

    /* interpolate within 2nd to last point */
    for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
    {
      coeffs = interp_coeff[fluid_phase_fract_to_tablerow (dsp_phase)];
      dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index-1]
				  + coeffs[1] * dsp_data[dsp_phase_index]
				  + coeffs[2] * dsp_data[dsp_phase_index+1]
				  + coeffs[3] * end_point1);

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index (dsp_phase);
      dsp_amp += dsp_amp_incr;
    }

    end_index++;	/* we're now interpolating the last point */

    /* interpolate within the last point */
    for (; dsp_phase_index <= end_index && dsp_i < FLUID_BUFSIZE; dsp_i++)
    {
      coeffs = interp_coeff[fluid_phase_fract_to_tablerow (dsp_phase)];
      dsp_buf[dsp_i] = dsp_amp * (coeffs[0] * dsp_data[dsp_phase_index-1]
				  + coeffs[1] * dsp_data[dsp_phase_index]
				  + coeffs[2] * end_point1
				  + coeffs[3] * end_point2);

      /* increment phase and amplitude */
      fluid_phase_incr (dsp_phase, dsp_phase_incr);
      dsp_phase_index = fluid_phase_index (dsp_phase);
      dsp_amp += dsp_amp_incr;
    }

    if (!looping) break;	/* break out if not looping (end of sample) */

    /* go back to loop start */
    if (dsp_phase_index > end_index)
    {
      fluid_phase_sub_int (dsp_phase, voice->loopend - voice->loopstart);

      if (!voice->has_looped)
      {
	voice->has_looped = 1;
	start_index = voice->loopstart;
	start_point = dsp_data[voice->loopend - 1];
      }
    }

    /* break out if filled buffer */
    if (dsp_i >= FLUID_BUFSIZE) break;

    end_index -= 2;	/* set end back to third to last sample point */
  }

  voice->phase = dsp_phase;
  voice->amp = dsp_amp;

  return (dsp_i);
}
