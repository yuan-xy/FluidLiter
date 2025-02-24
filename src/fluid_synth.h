#ifndef _FLUID_SYNTH_H
#define _FLUID_SYNTH_H

/***************************************************************
 *
 *                         INCLUDES
 */

#include "fluidsynth_priv.h"
#include "fluid_list.h"
#include "fluid_rev.h"
#include "fluid_chorus.h"
#include "fluid_voice.h"

/***************************************************************
 *
 *                         DEFINES
 */
#define FLUID_NUM_PROGRAMS 128
#define DRUM_INST_BANK 128

#define FLUID_CHORUS_DEFAULT_DEPTH 4.25f                 /**< Default chorus depth */
#define FLUID_CHORUS_DEFAULT_LEVEL 0.6f                  /**< Default chorus level */
#define FLUID_CHORUS_DEFAULT_N 3                         /**< Default chorus voice count */
#define FLUID_CHORUS_DEFAULT_SPEED 0.2f                  /**< Default chorus speed */
#define FLUID_CHORUS_DEFAULT_TYPE FLUID_CHORUS_MOD_SINE  /**< Default chorus waveform type */

/***************************************************************
 *
 *                         ENUM
 */
enum fluid_loop {
    FLUID_UNLOOPED = 0,
    FLUID_LOOP_DURING_RELEASE = 1,
    FLUID_NOTUSED = 2,
    FLUID_LOOP_UNTIL_RELEASE = 3
};

enum fluid_synth_status {
    FLUID_SYNTH_CLEAN,
    FLUID_SYNTH_PLAYING,
    FLUID_SYNTH_QUIET,
    FLUID_SYNTH_STOPPED
};

typedef struct _fluid_bank_offset_t fluid_bank_offset_t;

struct _fluid_bank_offset_t {
    int sfont_id;
    int offset;
};

struct _fluid_synth_t {
    int polyphony;      /** maximum polyphony */
    double sample_rate; /** The sample rate */
    int midi_channels;  /** the number of MIDI channels */
    unsigned int state; /** the synthesizer state */
    unsigned int ticks; /** the number of audio samples since the start */

    fluid_fileapi_t *fileapi; /** to load soundfont, remove sfloader_t type */
    fluid_list_t *sfont;   /** the loaded soundfont */
    unsigned int sfont_id;
    fluid_list_t *bank_offsets; /** the offsets of the soundfont banks */

    double gain;               /** master gain */
    fluid_channel_t **channel; /** the channels */
    int nvoice;                /** the length of the synthesis process array */
    fluid_voice_t **voice;     /** the synthesis processes */
    unsigned int noteid; /** the id is incremented for every new note. it's used
                            for noteoff's  */
    unsigned int storeid;

    /**< Shadow of chorus parameter: chorus number, level, speed, depth, type */
    double chorus_param[FLUID_CHORUS_PARAM_LAST];

    fluid_real_t *left_buf;
    fluid_real_t *right_buf;
    fluid_real_t *fx_left_buf;
    fluid_real_t *fx_right_buf;
    fluid_real_t *fx_left_buf2;
    fluid_real_t *fx_right_buf2;

    fluid_revmodel_t *reverb;
    fluid_chorus_t *chorus;
    int cur; /** the current sample in the audio buffers to be output */

    fluid_tuning_t ***tuning;   /** 128 banks of 128 programs for the tunings */
    fluid_tuning_t *cur_tuning; /** current tuning in the iteration */

    unsigned int
        min_note_length_ticks; /**< If note-offs are triggered just after a
                                  note-on, they will be delayed */
    bool with_reverb;   /** Should the synth use the built-in reverb unit? */
    bool with_chorus;
};

/** returns 1 if the value has been set, 0 otherwise */
int fluid_synth_setstr(fluid_synth_t *synth, char *name, char *str);

/** returns 1 if the value exists, 0 otherwise */
int fluid_synth_getstr(fluid_synth_t *synth, char *name, char **str);

/** returns 1 if the value has been set, 0 otherwise */
int fluid_synth_setnum(fluid_synth_t *synth, char *name, double val);

/** returns 1 if the value exists, 0 otherwise */
int fluid_synth_getnum(fluid_synth_t *synth, char *name, double *val);

/** returns 1 if the value has been set, 0 otherwise */
int fluid_synth_setint(fluid_synth_t *synth, char *name, int val);

/** returns 1 if the value exists, 0 otherwise */
int fluid_synth_getint(fluid_synth_t *synth, char *name, int *val);

int fluid_synth_set_reverb_preset(fluid_synth_t *synth, int num);

int fluid_synth_one_block(fluid_synth_t *synth, int do_not_mix_fx_to_out);

fluid_preset_t *fluid_synth_get_preset(fluid_synth_t *synth,
                                       unsigned int sfontnum,
                                       unsigned int banknum,
                                       unsigned int prognum);

fluid_preset_t *fluid_synth_find_preset(fluid_synth_t *synth,
                                        unsigned int banknum,
                                        unsigned int prognum);

int fluid_synth_all_notes_off(fluid_synth_t *synth, int chan);
int fluid_synth_all_sounds_off(fluid_synth_t *synth, int chan);
int fluid_synth_modulate_voices(fluid_synth_t *synth, int chan, int is_cc,
                                int ctrl);
int fluid_synth_modulate_voices_all(fluid_synth_t *synth, int chan);
int fluid_synth_damp_voices(fluid_synth_t *synth, int chan);
int fluid_synth_kill_voice(fluid_synth_t *synth, fluid_voice_t *voice);
void fluid_synth_kill_by_exclusive_class(fluid_synth_t *synth,
                                         fluid_voice_t *voice);
void fluid_synth_release_voice_on_same_note(fluid_synth_t *synth, int chan,
                                            int key);

/** This function assures that every MIDI channels has a valid preset
 *  (NULL is okay). This function is called after a SoundFont is
 *  unloaded or reloaded. */
void fluid_synth_update_presets(fluid_synth_t *synth);

int fluid_synth_update_gain(fluid_synth_t *synth, char *name, double value);
int fluid_synth_update_polyphony(fluid_synth_t *synth, char *name, int value);

fluid_bank_offset_t *fluid_synth_get_bank_offset0(fluid_synth_t *synth,
                                                  int sfont_id);
void fluid_synth_remove_bank_offset(fluid_synth_t *synth, int sfont_id);

int fluid_synth_program_select2(fluid_synth_t *synth, int chan,
                                char *sfont_name, unsigned int bank_num,
                                unsigned int preset_num);

fluid_sfont_t *fluid_synth_get_sfont_by_name(fluid_synth_t *synth, char *name);

int fluid_synth_set_gen2(fluid_synth_t *synth, int chan, int param, float value,
                         int normalized);
        

void fluid_synth_init(); // need by testing GEN_TABLE_RUNTIME



#endif /* _FLUID_SYNTH_H */
