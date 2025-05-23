#ifndef _FLUID_CHAN_H
#define _FLUID_CHAN_H

#include "fluidsynth_priv.h"
#include "fluid_midi.h"
#include "fluid_tuning.h"

struct _fluid_channel_t {
    int channum; /**< MIDI channel number */
    unsigned int sfontnum;
    unsigned int banknum;
    unsigned int prognum;
    fluid_preset_t *preset;
    fluid_synth_t *synth;
    char key_pressure[128];
    short channel_pressure;
    short pitch_bend;
    short pitch_wheel_sensitivity;

    /* controller values */
    short cc[128];

    /* cached values of last MSB values of MSB/LSB controllers */
    unsigned char bank_msb;
    int interp_method;

    /* the micro-tuning */
    fluid_tuning_t *tuning;

    /* NRPN system */
    short nrpn_select;
    short nrpn_active; /* 1 if data entry CCs are for NRPN, 0 if RPN */

    /* The values of the generators, set by NRPN messages, or by
     * fluid_synth_set_gen(), are cached in the channel so they can be
     * applied to future notes. They are copied to a voice's generators
     * in fluid_voice_init(), wihich calls fluid_gen_init().  */
    fluid_real_t gen[GEN_LAST];
};

fluid_channel_t *new_fluid_channel(fluid_synth_t *synth, int num);
int delete_fluid_channel(fluid_channel_t *chan);
void fluid_channel_init(fluid_channel_t *chan);
void fluid_channel_init_ctrl(fluid_channel_t *chan, int is_all_ctrl_off);
void fluid_channel_reset(fluid_channel_t *chan);
int fluid_channel_set_preset(fluid_channel_t *chan, fluid_preset_t *preset);
fluid_preset_t *fluid_channel_get_preset(fluid_channel_t *chan);
unsigned int fluid_channel_get_sfontnum(fluid_channel_t *chan);
int fluid_channel_set_sfontnum(fluid_channel_t *chan, unsigned int sfont);
unsigned int fluid_channel_get_banknum(fluid_channel_t *chan);
int fluid_channel_set_banknum(fluid_channel_t *chan, unsigned int bank);
int fluid_channel_set_prognum(fluid_channel_t *chan, int prognum);
int fluid_channel_get_prognum(fluid_channel_t *chan);
int fluid_channel_cc(fluid_channel_t *chan, int ctrl, int val);
int fluid_channel_pressure(fluid_channel_t *chan, int val);
int fluid_channel_pitch_bend(fluid_channel_t *chan, int val);
int fluid_channel_pitch_wheel_sens(fluid_channel_t *chan, int val);
int fluid_channel_get_cc(fluid_channel_t *chan, int num);
int fluid_channel_get_num(fluid_channel_t *chan);
void fluid_channel_set_interp_method(fluid_channel_t *chan, uint8_t new_method);
uint8_t fluid_channel_get_interp_method(fluid_channel_t *chan);

#define fluid_channel_get_key_pressure(chan, key) ((chan)->key_pressure[key])
#define fluid_channel_set_key_pressure(chan, key, val)                         \
    ((chan)->key_pressure[key] = (val))
#define fluid_channel_set_tuning(_c, _t)                                       \
    { (_c)->tuning = _t; }
#define fluid_channel_has_tuning(_c) ((_c)->tuning != NULL)
#define fluid_channel_get_tuning(_c) ((_c)->tuning)
#define fluid_channel_sustained(_c) ((_c)->cc[SUSTAIN_SWITCH] >= 64)
#define fluid_channel_set_gen(_c, _n, _v)                                  \
    {                                                                          \
        (_c)->gen[_n] = _v;                                                    \
    }
#define fluid_channel_get_gen(_c, _n) ((_c)->gen[_n])

#define fluid_channel_get_min_note_length_ticks(chan)                          \
    ((chan)->synth->min_note_length_ticks)

#endif /* _FLUID_CHAN_H */
