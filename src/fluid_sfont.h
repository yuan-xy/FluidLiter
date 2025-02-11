
#ifndef _PRIV_FLUID_SFONT_H
#define _PRIV_FLUID_SFONT_H

/*
 * Utility macros to access soundfonts, presets, and samples
 */

#define fluid_fileapi_delete(_fileapi)                                         \
    {                                                                          \
        if ((_fileapi) && (_fileapi)->free) (*(_fileapi)->free)(_fileapi);     \
    }
#define fluid_sfloader_delete(_loader)                                         \
    {                                                                          \
        if (_loader) {                                                         \
            fluid_fileapi_delete((_loader)->fileapi);                          \
            if ((_loader)->free) (*(_loader)->free)(_loader);                  \
        }                                                                      \
    }
#define fluid_sfloader_load(_loader, _filename)                                \
    (*(_loader)->load)(_loader, _filename)

#define delete_fluid_sfont(_sf)                                                \
    (((_sf) && (_sf)->free) ? (*(_sf)->free)(_sf) : 0)
#define fluid_sfont_get_name(_sf) (*(_sf)->get_name)(_sf)
#define fluid_sfont_get_preset(_sf, _bank, _prenum)                            \
    (*(_sf)->get_preset)(_sf, _bank, _prenum)
#define fluid_sfont_iteration_start(_sf) (*(_sf)->iteration_start)(_sf)
#define fluid_sfont_iteration_next(_sf, _pr) (*(_sf)->iteration_next)(_sf, _pr)
#define fluid_sfont_get_data(_sf) (_sf)->data
#define fluid_sfont_set_data(_sf, _p)                                          \
    { (_sf)->data = (void *)(_p); }

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

#define fluid_sample_incr_ref(_sample)                                         \
    { (_sample)->refcount++; }

#define fluid_sample_decr_ref(_sample)                                         \
    (_sample)->refcount--;                                                     

#endif /* _PRIV_FLUID_SFONT_H */
