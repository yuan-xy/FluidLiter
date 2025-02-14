#ifndef _FLUIDSYNTH_SFONT_H
#define _FLUIDSYNTH_SFONT_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_SFONT_H */
