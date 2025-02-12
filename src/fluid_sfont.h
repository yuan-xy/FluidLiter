
#ifndef _PRIV_FLUID_SFONT_H
#define _PRIV_FLUID_SFONT_H


#define delete_fluid_preset(_preset)                                           \
    {                                                                          \
        if ((_preset) && (_preset)->free) {                                    \
            (*(_preset)->free)(_preset);                                       \
        }                                                                      \
    }

#define fluid_preset_get_data(_preset) (_preset)->data
#define fluid_preset_set_data(_preset, _p)                                     \
    { (_preset)->data = (void *)(_p); }
#define fluid_preset_get_name(_preset) (*(_preset)->get_name)(_preset)
#define fluid_preset_get_banknum(_preset) (*(_preset)->get_banknum)(_preset)
#define fluid_preset_get_num(_preset) (*(_preset)->get_num)(_preset)

#define fluid_preset_noteon(_preset, _synth, _ch, _key, _vel)                  \
    (*(_preset)->noteon)(_preset, _synth, _ch, _key, _vel)

#endif /* _PRIV_FLUID_SFONT_H */
