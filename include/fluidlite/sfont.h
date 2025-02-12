#ifndef _FLUIDSYNTH_SFONT_H
#define _FLUIDSYNTH_SFONT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 *   SoundFont plugins
 *
 *    It is possible to add new SoundFont loaders to the
 *    synthesizer. The API uses a couple of "interfaces" (structures
 *    with callback functions): fluid_sfont_t, and
 *    fluid_preset_t.
 *
 *    The fluid_sfont_t structure contains a callback to obtain the
 *    name of the soundfont. It contains two functions to iterate
 *    though the contained presets, and one function to obtain a
 *    preset corresponding to a bank and preset number. This
 *    function should return an fluid_preset_t structure.
 *
 *    The fluid_preset_t structure contains some functions to obtain
 *    information from the preset (name, bank, number). The most
 *    important callback is the noteon function. The noteon function
 *    should call fluid_synth_alloc_voice() for every sample that has
 *    to be played. fluid_synth_alloc_voice() expects a pointer to a
 *    fluid_sample_t structure and returns a pointer to the opaque
 *    fluid_voice_t structure. To set or increments the values of a
 *    generator, use fluid_voice_gen_{set,incr}. When you are
 *    finished initializing the voice call fluid_voice_start() to
 *    start playing the synthesis voice.
 * */


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

void fluid_init_default_fileapi(fluid_fileapi_t *fileapi);

void fluid_set_default_fileapi(fluid_fileapi_t *fileapi);


struct _fluid_sfont_t {
    void *data;
    unsigned int id;

    /** The 'free' callback function should return 0 when it was able to
        free all resources. It should return a non-zero value if some of
        the samples could not be freed because they are still in use. */
    int (*free)(fluid_sfont_t *sfont);

    /** Return the name of the sfont */
    char *(*get_name)(fluid_sfont_t *sfont);

    /** Return the preset with the specified bank and preset number. All
     *  the fields, including the 'sfont' field, should * be filled
     *  in. If the preset cannot be found, the function returns NULL. */
    fluid_preset_t *(*get_preset)(fluid_sfont_t *sfont, unsigned int bank,
                                  unsigned int prenum);

    void (*iteration_start)(fluid_sfont_t *sfont);

    /* return 0 when no more presets are available, 1 otherwise */
    int (*iteration_next)(fluid_sfont_t *sfont, fluid_preset_t *preset);
};

#define fluid_sfont_get_id(_sf) ((_sf)->id)

struct _fluid_preset_t {
    void *data;
    fluid_sfont_t *sfont;
    int (*free)(fluid_preset_t *preset);
    char *(*get_name)(fluid_preset_t *preset);
    int (*get_banknum)(fluid_preset_t *preset);
    int (*get_num)(fluid_preset_t *preset);

    /** handle a noteon event. Returns 0 if no error occured. */
    int (*noteon)(fluid_preset_t *preset, fluid_synth_t *synth, int chan,
                  int key, int vel);
};

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

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SFONT_H */
