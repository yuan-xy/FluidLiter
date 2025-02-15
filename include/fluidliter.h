#ifndef _FLUIDLITE_H
#define _FLUIDLITE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif

typedef enum { FLUID_OK = 0, FLUID_FAILED = -1 } fluid_status;

/*

  Forward declarations

*/
typedef struct _fluid_synth_t fluid_synth_t;
typedef struct _fluid_voice_t fluid_voice_t;
typedef struct _fluid_sfont_t fluid_sfont_t;
typedef struct _fluid_preset_t fluid_preset_t;
typedef struct _fluid_sample_t fluid_sample_t;
typedef struct _fluid_mod_t fluid_mod_t;
typedef struct _fluid_midi_event_t fluid_midi_event_t;
typedef struct _fluid_hashtable_t fluid_cmd_handler_t;
typedef struct _fluid_event_t fluid_event_t;
typedef struct _fluid_fileapi_t fluid_fileapi_t;


// #include "synth.h"

/**   Embedded synthesizer
 *
 *    You create a new synthesizer with new_fluid_synth() and you destroy
 *    if with delete_fluid_synth(). Use the settings structure to specify
 *    the synthesizer characteristics.
 *
 *    You have to load a SoundFont in order to hear any sound. For that
 *    you use the fluid_synth_sfload() function.
 * 
 *    The API for sending MIDI events is probably what you expect:
 *    fluid_synth_noteon(), fluid_synth_noteoff(), ...
 *
 */

 typedef struct {
    int polyphony;
    double gain;
    double sample_rate;
    bool with_reverb;
    bool with_chorus;
    int midi_channels;
} SynthParams;

/** Creates a new synthesizer object.
 *
 * \return a newly allocated synthesizer or NULL in case of error
 */
fluid_synth_t *new_fluid_synth(SynthParams sp);

#define NEW_FLUID_SYNTH(...)                                                   \
    new_fluid_synth((SynthParams){.polyphony = 10,                             \
                                  .gain = 0.4,                                 \
                                  .sample_rate = 44100.0,                      \
                                  .with_reverb = true,                         \
                                  .with_chorus = false,                         \
                                  .midi_channels = 1,                          \
                                  __VA_ARGS__})

void fluid_synth_set_sample_rate(fluid_synth_t *synth,
                                                float sample_rate);

/**
 * Deletes the synthesizer previously created with new_fluid_synth.
 *
 * \param synth the synthesizer object
 * \return 0 if no error occured, -1 otherwise
 */
int delete_fluid_synth(fluid_synth_t *synth);

/*
 *
 * MIDI channel messages
 *
 */

/** Send a noteon message. Returns 0 if no error occurred, -1 otherwise. */
int fluid_synth_noteon(fluid_synth_t *synth, int chan, int key,
                                      int vel);

/** Send a noteoff message. Returns 0 if no error occurred, -1 otherwise.  */
int fluid_synth_noteoff(fluid_synth_t *synth, int chan, int key);

/** Send a control change message. Returns 0 if no error occurred, -1 otherwise.
 */
int fluid_synth_cc(fluid_synth_t *synth, int chan, int ctrl,
                                  int val);

/** Get a control value. Returns 0 if no error occurred, -1 otherwise.  */
int fluid_synth_get_cc(fluid_synth_t *synth, int chan, int ctrl,
                                      int *pval);

/** Send a pitch bend message. Returns 0 if no error occurred, -1 otherwise.  */
int fluid_synth_pitch_bend(fluid_synth_t *synth, int chan,
                                          int val);

/** Get the pitch bend value. Returns 0 if no error occurred, -1 otherwise. */

int fluid_synth_get_pitch_bend(fluid_synth_t *synth, int chan,
                               int *ppitch_bend);

/** Set the pitch wheel sensitivity. Returns 0 if no error occurred, -1
 * otherwise. */
int fluid_synth_pitch_wheel_sens(fluid_synth_t *synth, int chan,
                                                int val);

/** Get the pitch wheel sensitivity. Returns 0 if no error occurred, -1
 * otherwise. */
int fluid_synth_get_pitch_wheel_sens(fluid_synth_t *synth,
                                                    int chan, int *pval);

/** Send a program change message. Returns 0 if no error occurred, -1 otherwise.
 */
int fluid_synth_program_change(fluid_synth_t *synth, int chan,
                                              int program);

int fluid_synth_channel_pressure(fluid_synth_t *synth, int chan,
                                                int val);
int fluid_synth_key_pressure(fluid_synth_t *synth, int chan,
                                            int key, int val);
int fluid_synth_sysex(fluid_synth_t *synth, const char *data,
                                     int len, char *response, int *response_len,
                                     int *handled, int dryrun);

/** Select a bank. Returns 0 if no error occurred, -1 otherwise. */

int fluid_synth_bank_select(fluid_synth_t *synth, int chan, unsigned int bank);

/** Select a sfont. Returns 0 if no error occurred, -1 otherwise. */

int fluid_synth_sfont_select(fluid_synth_t *synth, int chan,
                             unsigned int sfont_id);

/** Select a preset for a channel. The preset is specified by the
    SoundFont ID, the bank number, and the preset number. This
    allows any preset to be selected and circumvents preset masking
    due to previously loaded SoundFonts on the SoundFont stack.

    \param synth The synthesizer
    \param chan The channel on which to set the preset
    \param sfont_id The ID of the SoundFont
    \param bank_num The bank number
    \param preset_num The preset number
    \return 0 if no errors occured, -1 otherwise
*/

int fluid_synth_program_select(fluid_synth_t *synth, int chan,
                               unsigned int sfont_id, unsigned int bank_num,
                               unsigned int preset_num);

/** Returns the program, bank, and SoundFont number of the preset on
    a given channel. Returns 0 if no error occurred, -1 otherwise. */

int fluid_synth_get_program(fluid_synth_t *synth, int chan,
                            unsigned int *sfont_id, unsigned int *bank_num,
                            unsigned int *preset_num);

/** Send a bank select and a program change to every channel to
 *  reinitialize the preset of the channel. This function is useful
 *  mainly after a SoundFont has been loaded, unloaded or
 *  reloaded. . Returns 0 if no error occurred, -1 otherwise. */
int fluid_synth_program_reset(fluid_synth_t *synth);

/** Send a reset. A reset turns all the notes off and resets the
    controller values. */
int fluid_synth_system_reset(fluid_synth_t *synth);

/*
 *
 * Low level access
 *
 */

/** Create and start voices using a preset. The id passed as
 * argument will be used as the voice group id.  */
int fluid_synth_start(fluid_synth_t *synth, unsigned int id,
                                     fluid_preset_t *preset, int audio_chan,
                                     int midi_chan, int key, int vel);

/** Stop the voices in the voice group defined by id. */
int fluid_synth_stop(fluid_synth_t *synth, unsigned int id);

/** Change the value of a generator of the voices in the voice group
 * defined by id. */
/* int fluid_synth_ctrl(fluid_synth_t* synth, int id,  */
/* 				    int gen, float value,  */
/* 				    int absolute, int normalized); */

/*
 *
 * SoundFont management
 *
 */

/** Set an optional function callback each time a preset has finished loading.
    This can be useful when calling fluid_synth_sfload asynchronously.
    The function must be formatted like this:
    void my_callback_function(int bank, int num, char* name)

    \param callback Pointer to the function
*/

void fluid_synth_set_preset_callback(void *callback);

/** Loads a SoundFont file and creates a new SoundFont. The newly
    loaded SoundFont will be put on top of the SoundFont
    stack. Presets are searched starting from the SoundFont on the
    top of the stack, working the way down the stack until a preset
    is found.

    \param synth The synthesizer object
    \param filename The file name
    \param reset_presets If non-zero, the presets on the channels will be reset
    \returns The ID of the loaded SoundFont, or -1 in case of error
*/

int fluid_synth_sfload(fluid_synth_t *synth, const char *filename,
                       int reset_presets);


/** Removes a SoundFont from the stack and deallocates it.

    \param synth The synthesizer object
    \param id The id of the SoundFont
    \param reset_presets If TRUE then presets will be reset for all channels
    \returns 0 if no error, -1 otherwise
*/
int fluid_synth_sfunload(fluid_synth_t *synth, unsigned int id,
                                        int reset_presets);

/** Add a SoundFont. The SoundFont will be put on top of
    the SoundFont stack.

    \param synth The synthesizer object
    \param sfont The SoundFont
    \returns The ID of the loaded SoundFont, or -1 in case of error
*/
int fluid_synth_add_sfont(fluid_synth_t *synth,
                                         fluid_sfont_t *sfont);

/** Remove a SoundFont that was previously added using
 *  fluid_synth_add_sfont(). The synthesizer does not delete the
 *  SoundFont; this is responsability of the caller.

    \param synth The synthesizer object
    \param sfont The SoundFont
*/
void fluid_synth_remove_sfont(fluid_synth_t *synth,
                                             fluid_sfont_t *sfont);

/** Count the number of loaded SoundFonts.

    \param synth The synthesizer object
    \returns The number of loaded SoundFonts
*/
int fluid_synth_sfcount(fluid_synth_t *synth);

/** Get a SoundFont. The SoundFont is specified by its index on the
    stack. The top of the stack has index zero.

    \param synth The synthesizer object
    \param num The number of the SoundFont (0 <= num < sfcount)
    \returns A pointer to the SoundFont
*/
fluid_sfont_t *fluid_synth_get_sfont(fluid_synth_t *synth,
                                                    unsigned int num);

/** Get a SoundFont. The SoundFont is specified by its ID.

    \param synth The synthesizer object
    \param id The id of the sfont
    \returns A pointer to the SoundFont
*/
fluid_sfont_t *fluid_synth_get_sfont_by_id(fluid_synth_t *synth,
                                                          unsigned int id);

/** Get the preset of a channel */
fluid_preset_t *
fluid_synth_get_channel_preset(fluid_synth_t *synth, int chan);

/** Offset the bank numbers in a SoundFont. Returns -1 if an error
 * occured (out of memory or negative offset) */
int fluid_synth_set_bank_offset(fluid_synth_t *synth,
                                               int sfont_id, int offset);

/** Get the offset of the bank numbers in a SoundFont. */
int fluid_synth_get_bank_offset(fluid_synth_t *synth,
                                               int sfont_id);

/*
 *
 * Reverb
 *
 */

/** Set the parameters for the built-in reverb unit */
void fluid_synth_set_reverb(fluid_synth_t *synth,
                                           double roomsize, double damping,
                                           double width, double level);

/** Turn on (1) / off (0) the built-in reverb unit */
void fluid_synth_set_reverb_on(fluid_synth_t *synth, int on);

/** Query the current state of the reverb. */
double fluid_synth_get_reverb_roomsize(fluid_synth_t *synth);
double fluid_synth_get_reverb_damp(fluid_synth_t *synth);
double fluid_synth_get_reverb_level(fluid_synth_t *synth);
double fluid_synth_get_reverb_width(fluid_synth_t *synth);

/* Those are the default settings for the reverb */
#define FLUID_REVERB_DEFAULT_DAMP 0.3f      /**< Default reverb damping */
#define FLUID_REVERB_DEFAULT_LEVEL 0.7f     /**< Default reverb level */
#define FLUID_REVERB_DEFAULT_ROOMSIZE 0.5f  /**< Default reverb room size */
#define FLUID_REVERB_DEFAULT_WIDTH 0.8f     /**< Default reverb width */

/**
 * Chorus modulation waveform type.
 */
enum fluid_chorus_mod
{
    FLUID_CHORUS_MOD_SINE = 0,            /**< Sine wave chorus modulation */
    FLUID_CHORUS_MOD_TRIANGLE = 1         /**< Triangle wave chorus modulation */
};

#define FLUID_CHORUS_DEFAULT_DEPTH 4.25f                 /**< Default chorus depth */
#define FLUID_CHORUS_DEFAULT_LEVEL 0.6f                  /**< Default chorus level */
#define FLUID_CHORUS_DEFAULT_N 3                         /**< Default chorus voice count */
#define FLUID_CHORUS_DEFAULT_SPEED 0.2f                  /**< Default chorus speed */
#define FLUID_CHORUS_DEFAULT_TYPE FLUID_CHORUS_MOD_SINE  /**< Default chorus waveform type */



/** Returns the number of MIDI channels that the synthesizer uses
    internally */
int fluid_synth_count_midi_channels(fluid_synth_t *synth);

/** Returns the number of effects channels that the synthesizer uses
    internally */
int fluid_synth_count_effects_channels(fluid_synth_t *synth);

/*
 *
 * Synthesis parameters
 *
 */

/** Set the master gain */
void fluid_synth_set_gain(fluid_synth_t *synth, float gain);

/** Get the master gain */
float fluid_synth_get_gain(fluid_synth_t *synth);

/** Set the polyphony limit (FluidSynth >= 1.0.6) */
int fluid_synth_set_polyphony(fluid_synth_t *synth,
                                             int polyphony);

/** Get the polyphony limit (FluidSynth >= 1.0.6) */
int fluid_synth_get_polyphony(fluid_synth_t *synth);

/** Get the internal buffer size. The internal buffer size if not the
    same thing as the buffer size specified in the
    settings. Internally, the synth *always* uses a specific buffer
    size independent of the buffer size used by the audio driver. The
    internal buffer size is normally 64 samples. The reason why it
    uses an internal buffer size is to allow audio drivers to call the
    synthesizer with a variable buffer length. The internal buffer
    size is useful for client who want to optimize their buffer sizes.
*/
int fluid_synth_get_internal_bufsize(fluid_synth_t *synth);

/** Set the interpolation method for one channel or all channels (chan = -1) */

int fluid_synth_set_interp_method(fluid_synth_t *synth, int chan,
                                  uint8_t interp_method);

/* Flags to choose the interpolation method */
enum fluid_interp {
    /* no interpolation: Fastest, but questionable audio quality */
    FLUID_INTERP_NONE = 0,
    /* Straight-line interpolation: A bit slower, reasonable audio quality */
    FLUID_INTERP_LINEAR = 1,
    /* Fourth-order interpolation: Requires 50 % of the whole DSP processing
     * time, good quality Default. */
    FLUID_INTERP_DEFAULT = 4,
    FLUID_INTERP_4THORDER = 4,
};

/*
 *
 * Generator interface
 *
 */

/** Change the value of a generator. This function allows to control
    all synthesis parameters in real-time. The changes are additive,
    i.e. they add up to the existing parameter value. This function is
    similar to sending an NRPN message to the synthesizer. The
    function accepts a float as the value of the parameter. The
    parameter numbers and ranges are described in the SoundFont 2.01
    specification, paragraph 8.1.3, page 48. See also 'fluid_gen_type'.

    \param synth The synthesizer object.
    \param chan The MIDI channel number.
    \param param The parameter number.
    \param value The parameter value.
    \returns Your favorite dish.
*/

int fluid_synth_set_gen(fluid_synth_t *synth, int chan, int param, float value);

/** Retreive the value of a generator. This function returns the value
    set by a previous call 'fluid_synth_set_gen' or by an NRPN message.

    \param synth The synthesizer object.
    \param chan The MIDI channel number.
    \param param The generator number.
    \returns The value of the generator.
*/
float fluid_synth_get_gen(fluid_synth_t *synth, int chan,
                                         int param);

/*
 *
 * Tuning
 *
 */

/** Create a new key-based tuning with given name, number, and
    pitches. The array 'pitches' should have length 128 and contains
    the pitch in cents of every key in cents. However, if 'pitches' is
    NULL, a new tuning is created with the well-tempered scale.

    \param synth The synthesizer object
    \param tuning_bank The tuning bank number [0-127]
    \param tuning_prog The tuning program number [0-127]
    \param name The name of the tuning
    \param pitch The array of pitch values. The array length has to be 128.
*/

int fluid_synth_create_key_tuning(fluid_synth_t *synth, int tuning_bank,
                                  int tuning_prog, const char *name,
                                  double *pitch);

/** Create a new octave-based tuning with given name, number, and
    pitches.  The array 'pitches' should have length 12 and contains
    derivation in cents from the well-tempered scale. For example, if
    pitches[0] equals -33, then the C-keys will be tuned 33 cents
    below the well-tempered C.

    \param synth The synthesizer object
    \param tuning_bank The tuning bank number [0-127]
    \param tuning_prog The tuning program number [0-127]
    \param name The name of the tuning
    \param pitch The array of pitch derivations. The array length has to be 12.
*/

int fluid_synth_create_octave_tuning(fluid_synth_t *synth, int tuning_bank,
                                     int tuning_prog, const char *name,
                                     const double *pitch);


int fluid_synth_activate_octave_tuning(fluid_synth_t *synth, int bank, int prog,
                                       const char *name, const double *pitch,
                                       int apply);

/** Request a note tuning changes. Both they 'keys' and 'pitches'
    arrays should be of length 'num_pitches'. If 'apply' is non-zero,
    the changes should be applied in real-time, i.e. sounding notes
    will have their pitch updated. 'APPLY' IS CURRENTLY IGNORED. The
    changes will be available for newly triggered notes only.

    \param synth The synthesizer object
    \param tuning_bank The tuning bank number [0-127]
    \param tuning_prog The tuning program number [0-127]
    \param len The length of the keys and pitch arrays
    \param keys The array of keys values.
    \param pitch The array of pitch values.
    \param apply Flag to indicate whether to changes should be applied in
   real-time.
*/

int fluid_synth_tune_notes(fluid_synth_t *synth, int tuning_bank,
                           int tuning_prog, int len, int *keys, double *pitch,
                           int apply);

/** Select a tuning for a channel.

\param synth The synthesizer object
\param chan The channel number [0-max channels]
\param tuning_bank The tuning bank number [0-127]
\param tuning_prog The tuning program number [0-127]
*/

int fluid_synth_select_tuning(fluid_synth_t *synth, int chan, int tuning_bank,
                              int tuning_prog);

int fluid_synth_activate_tuning(fluid_synth_t *synth, int chan, int bank,
                                int prog, int apply);

/** Set the tuning to the default well-tempered tuning on a channel.

\param synth The synthesizer object
\param chan The channel number [0-max channels]
*/
int fluid_synth_reset_tuning(fluid_synth_t *synth, int chan);

/** Start the iteration throught the list of available tunings.

\param synth The synthesizer object
*/
void fluid_synth_tuning_iteration_start(fluid_synth_t *synth);

/** Get the next tuning in the iteration. This functions stores the
    bank and program number of the next tuning in the pointers given as
    arguments.

    \param synth The synthesizer object
    \param bank Pointer to an int to store the bank number
    \param prog Pointer to an int to store the program number
    \returns 1 if there is a next tuning, 0 otherwise
*/

int fluid_synth_tuning_iteration_next(fluid_synth_t *synth, int *bank,
                                      int *prog);

/** Dump the data of a tuning. This functions stores the name and
    pitch values of a tuning in the pointers given as arguments. Both
    name and pitch can be NULL is the data is not needed.

    \param synth The synthesizer object
    \param bank The tuning bank number [0-127]
    \param prog The tuning program number [0-127]
    \param name Pointer to a buffer to store the name
    \param len The length of the name buffer
    \param pitch Pointer to buffer to store the pitch values
*/
int fluid_synth_tuning_dump(fluid_synth_t *synth, int bank,
                                           int prog, char *name, int len,
                                           double *pitch);

/** Generate a number of samples. This function expects two signed
 *  16bits buffers (left and right channel) that will be filled with
 *  samples.
 *
 *  \param synth The synthesizer
 *  \param len The number of samples to generate
 *  \param lout The sample buffer for the left channel
 *  \param loff The offset, in samples, in the left buffer where the writing
 * pointer starts \param lincr The increment, in samples, of the writing pointer
 * in the left buffer \param rout The sample buffer for the right channel \param
 * roff The offset, in samples, in the right buffer where the writing pointer
 * starts \param rincr The increment, in samples, of the writing pointer in the
 * right buffer \returns 0 if no error occured, non-zero otherwise
 */

int fluid_synth_write_s16(fluid_synth_t *synth, int len,
                                         void *lout, int loff, int lincr,
                                         void *rout, int roff, int rincr);

int fluid_synth_write_s16_mono(fluid_synth_t *synth, int len,
                                              void *out);

int fluid_synth_write_u12(fluid_synth_t *synth, int len,
                                         uint16_t *out, int channel);

int fluid_synth_write_u12_mono(fluid_synth_t *synth, int len,
                                              uint16_t *out);

int fluid_synth_write_u8(fluid_synth_t *synth, int len,
                                        uint8_t *out, int channel);

int fluid_synth_write_u8_mono(fluid_synth_t *synth, int len,
                                             uint8_t *out);

/** Generate a number of samples. This function expects two floating
 *  point buffers (left and right channel) that will be filled with
 *  samples.
 *
 *  \param synth The synthesizer
 *  \param len The number of samples to generate
 *  \param lout The sample buffer for the left channel
 *  \param loff The offset, in samples, in the left buffer where the writing
 * pointer starts \param lincr The increment, in samples, of the writing pointer
 * in the left buffer \param rout The sample buffer for the right channel \param
 * roff The offset, in samples, in the right buffer where the writing pointer
 * starts \param rincr The increment, in samples, of the writing pointer in the
 * right buffer \returns 0 if no error occured, non-zero otherwise
 */

int fluid_synth_write_float(fluid_synth_t *synth, int len,
                                           void *lout, int loff, int lincr,
                                           void *rout, int roff, int rincr);


/** Allocate a synthesis voice. This function is called by a
    soundfont's preset in response to a noteon event.
    The returned voice comes with default modulators installed
   (velocity-to-attenuation, velocity to filter, ...) Note: A single noteon
   event may create any number of voices, when the preset is layered. Typically
   1 (mono) or 2 (stereo).*/
fluid_voice_t *fluid_synth_alloc_voice(fluid_synth_t *synth,
                                                      fluid_sample_t *sample,
                                                      int channum, int key,
                                                      int vel);

/** Start a synthesis voice. This function is called by a
    soundfont's preset in response to a noteon event after the voice
    has been allocated with fluid_synth_alloc_voice() and
    initialized.
    Exclusive classes are processed here.*/
void fluid_synth_start_voice(fluid_synth_t *synth,
                                            fluid_voice_t *voice);

/** Write a list of all voices matching ID into buf, but not more than bufsize
 * voices. If ID <0, return all voices. */
void fluid_synth_get_voicelist(fluid_synth_t *synth,
                                              fluid_voice_t *buf[], int bufsize,
                                              int ID);



// #include "sfont.h"

/**
 * File callback structure to enable custom soundfont loading (e.g. from
 * memory).
 */
struct _fluid_fileapi_t {
    /**
     * The free must free the memory allocated for the loader in
     * addition to any private data. It should return 0 if no error
     * occured, non-zero otherwise.
     */
    int (*free)(fluid_fileapi_t *fileapi);

    /**
     * Opens the file or memory indicated by \c filename in binary read mode.
     * \c filename matches the one provided during the fluid_synth_sfload()
     * call.
     *
     * @return returns a file handle on success, NULL otherwise
     */
    void *(*fopen)(fluid_fileapi_t *fileapi, const char *filename);

    /**
     * Reads \c count bytes to the specified buffer \c buf.
     *
     * @return returns #FLUID_OK if exactly \c count bytes were successfully
     * read, else #FLUID_FAILED
     */
    int (*fread)(void *buf, int count, void *handle);

    void *(*fread_zero_memcpy)(int count, void *handle);

    /**
     * Same purpose and behaviour as fseek.
     *
     * @param origin either \c SEEK_SET, \c SEEK_CUR or \c SEEK_END
     *
     * @return returns #FLUID_OK if the seek was successfully performed while
     * not seeking beyond a buffer or file, #FLUID_FAILED otherwise */
    int (*fseek)(void *handle, long offset, int origin);

    /**
     * Closes the handle and frees used ressources.
     *
     * @return returns #FLUID_OK on success, #FLUID_FAILED on error */
    int (*fclose)(void *handle);

    /** @return returns current file offset or #FLUID_FAILED on error */
    long (*ftell)(void *handle);
};

void fluid_set_default_fileapi(fluid_fileapi_t *fileapi);

#define fluid_fileapi_delete(_fileapi)                                         \
    {                                                                          \
        if ((_fileapi) && (_fileapi)->free) (*(_fileapi)->free)(_fileapi);     \
    }

    
#define fluid_sfont_get_id(_sf) ((_sf)->id)


struct _fluid_sample_t {
    char name[21];
    unsigned int start;
    unsigned int
        end; /* Note: Index of last valid sample point (contrary to SF spec) */
    unsigned int loopstart;
    unsigned int loopend; /* Note: first point following the loop (superimposed
                             on loopstart) */
    unsigned int samplerate;
    int origpitch;
    int pitchadj;
    int sampletype;
    int valid;
    short *data;

    /** The amplitude, that will lower the level of the sample's loop to
        the noise floor. Needed for note turnoff optimization, will be
        filled out automatically */
    /* Set this to zero, when submitting a new sample. */
    int amplitude_that_reaches_noise_floor_is_valid;
    double amplitude_that_reaches_noise_floor;
};


/** Sample types */

#define FLUID_SAMPLETYPE_MONO 1
#define FLUID_SAMPLETYPE_RIGHT 2
#define FLUID_SAMPLETYPE_LEFT 4
#define FLUID_SAMPLETYPE_LINKED 8
#define FLUID_SAMPLETYPE_OGG_VORBIS                                            \
    0x10 /**< Flag for #fluid_sample_t \a sampletype field for Ogg Vorbis      \
            compressed samples */
#define FLUID_SAMPLETYPE_OGG_VORBIS_UNPACKED 0x20




// #include "log.h"

/**
 * FluidSynth log levels.
 */
enum fluid_log_level {
    FLUID_PANIC = 0, /**< The synth can't function correctly any more */
    FLUID_ERR,       /**< Serious error occurred */
    FLUID_WARN,      /**< Warning */
    FLUID_INFO,      /**< Verbose informational messages */
    FLUID_DBG,       /**< Debugging messages */
    LAST_LOG_LEVEL
};

enum fluid_log_level get_log_level(void);
void set_log_level(enum fluid_log_level level);

int fluid_log(enum fluid_log_level level, char *fmt, ...);



// #include "mod.h"

/* Modulator-related definitions */

/* Maximum number of modulators in a voice */
#define FLUID_NUM_MOD 9 // only support default modulators


struct _fluid_mod_t {
    unsigned char dest;
    unsigned char src1;
    unsigned char flags1;
    unsigned char src2;
    unsigned char flags2;
    fluid_real_t amount;
};

struct _fluid_mod_list_t {
    unsigned char dest;
    unsigned char src1;
    unsigned char flags1;
    unsigned char src2;
    unsigned char flags2;
    fluid_real_t amount;
    struct _fluid_mod_list_t *next;
};
typedef struct _fluid_mod_list_t fluid_mod_list_t;

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
fluid_mod_list_t *fluid_mod_list_new(void);

/* Frees the modulator */
void fluid_mod_list_delete(fluid_mod_list_t *mod);

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




// #include "gen.h"

/**
 * @file gen.h
 * @brief Functions and defines for SoundFont generator effects.
 */

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum fluid_gen_type {
    GEN_STARTADDROFS,     /**< Sample start address offset (0-32767) */
    GEN_ENDADDROFS,       /**< Sample end address offset (-32767-0) */
    GEN_STARTLOOPADDROFS, /**< Sample loop start address offset (-32767-32767)
                           */
    GEN_ENDLOOPADDROFS,   /**< Sample loop end address offset (-32767-32767) */
    GEN_STARTADDRCOARSEOFS, /**< Sample start address coarse offset (X 32768) */
    GEN_MODLFOTOPITCH,      /**< Modulation LFO to pitch */
    GEN_VIBLFOTOPITCH,      /**< Vibrato LFO to pitch */
    GEN_MODENVTOPITCH,      /**< Modulation envelope to pitch */
    GEN_FILTERFC,           /**< Filter cutoff */
    GEN_FILTERQ,            /**< Filter Q */
    GEN_MODLFOTOFILTERFC,   /**< Modulation LFO to filter cutoff */
    GEN_MODENVTOFILTERFC,   /**< Modulation envelope to filter cutoff */
    GEN_ENDADDRCOARSEOFS,   /**< Sample end address coarse offset (X 32768) */
    GEN_MODLFOTOVOL,        /**< Modulation LFO to volume */
    GEN_UNUSED1,            /**< Unused */
    GEN_CHORUSSEND,         /**< Chorus send amount */
    GEN_REVERBSEND,         /**< Reverb send amount */
    GEN_PAN,                /**< Stereo panning */
    GEN_UNUSED2,            /**< Unused */
    GEN_UNUSED3,            /**< Unused */
    GEN_UNUSED4,            /**< Unused */
    GEN_MODLFODELAY,        /**< Modulation LFO delay */
    GEN_MODLFOFREQ,         /**< Modulation LFO frequency */
    GEN_VIBLFODELAY,        /**< Vibrato LFO delay */
    GEN_VIBLFOFREQ,         /**< Vibrato LFO frequency */
    GEN_MODENVDELAY,        /**< Modulation envelope delay */
    GEN_MODENVATTACK,       /**< Modulation envelope attack */
    GEN_MODENVHOLD,         /**< Modulation envelope hold */
    GEN_MODENVDECAY,        /**< Modulation envelope decay */
    GEN_MODENVSUSTAIN,      /**< Modulation envelope sustain */
    GEN_MODENVRELEASE,      /**< Modulation envelope release */
    GEN_KEYTOMODENVHOLD,    /**< Key to modulation envelope hold */
    GEN_KEYTOMODENVDECAY,   /**< Key to modulation envelope decay */
    GEN_VOLENVDELAY,        /**< Volume envelope delay */
    GEN_VOLENVATTACK,       /**< Volume envelope attack */
    GEN_VOLENVHOLD,         /**< Volume envelope hold */
    GEN_VOLENVDECAY,        /**< Volume envelope decay */
    GEN_VOLENVSUSTAIN,      /**< Volume envelope sustain */
    GEN_VOLENVRELEASE,      /**< Volume envelope release */
    GEN_KEYTOVOLENVHOLD,    /**< Key to volume envelope hold */
    GEN_KEYTOVOLENVDECAY,   /**< Key to volume envelope decay */
    GEN_INSTRUMENT,         /**< Instrument ID (shouldn't be set by user) */
    GEN_RESERVED1,          /**< Reserved */
    GEN_KEYRANGE,           /**< MIDI note range */
    GEN_VELRANGE,           /**< MIDI velocity range */
    GEN_STARTLOOPADDRCOARSEOFS, /**< Sample start loop address coarse offset (X
                                   32768) */
    GEN_KEYNUM,                 /**< Fixed MIDI note number */
    GEN_VELOCITY,               /**< Fixed MIDI velocity value */
    GEN_ATTENUATION,            /**< Initial volume attenuation */
    GEN_RESERVED2,              /**< Reserved */
    GEN_ENDLOOPADDRCOARSEOFS,   /**< Sample end loop address coarse offset (X
                                   32768) */
    GEN_COARSETUNE,             /**< Coarse tuning */
    GEN_FINETUNE,               /**< Fine tuning */
    GEN_SAMPLEID,               /**< Sample ID (shouldn't be set by user) */
    GEN_SAMPLEMODE,             /**< Sample mode flags */
    GEN_RESERVED3,              /**< Reserved */
    GEN_SCALETUNE,              /**< Scale tuning */
    GEN_EXCLUSIVECLASS,         /**< Exclusive class number */
    GEN_OVERRIDEROOTKEY,        /**< Sample root note override */

    /* the initial pitch is not a "standard" generator. It is not
     * mentioned in the list of generator in the SF2 specifications. It
     * is used, however, as the destination for the default pitch wheel
     * modulator. */
    GEN_PITCH, /**< Pitch (NOTE: Not a real SoundFont generator) */
    GEN_LAST   /**< Value defines the count of generators (#fluid_gen_type) */
};

/**
 * SoundFont generator structure.
 */
typedef struct _fluid_sf_gen_t {
    uint8_t num;
    fluid_real_t val;          /**< The nominal value */
} fluid_sf_gen_t; //TODO：similar to SFGen


typedef struct _fluid_gen_t {
    fluid_real_t val;          /**< The nominal value */
    fluid_real_t mod;          /**< Change by modulators */
    fluid_real_t nrpn;         /**< Change by NRPN messages */
} fluid_gen_t;


int fluid_gen_set_default_values(fluid_gen_t *gen);




// #include "voice.h"

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
   




// #include "music.h"

#define NN1 60
#define NOTE_C 60
#define NOTE_Db 61
#define NN2 62
#define NOTE_D 62
#define NOTE_Eb 63
#define NN3 64
#define NOTE_E 64
#define NN4 65
#define NOTE_F 65
#define NOTE_Gb 66
#define NN5 67
#define NOTE_G 67
#define NOTE_Ab 68
#define NN6 69
#define NOTE_A 69
#define NOTE_Bb 70
#define NN7 71
#define NOTE_B 71

// misc.c
// 计算音量（dB）
float calculateVolumeDB(int16_t *pcmData, int length);

float calculate_peak_dB(int16_t *pcmData, int length);

float calculate_peak_dB_1024(int16_t *pcmData, int length);


#ifdef __cplusplus
}
#endif

#endif /* _FLUIDLITE_H */
