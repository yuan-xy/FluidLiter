#ifndef _FLUID_GEN_H
#define _FLUID_GEN_H

#include "fluidsynth_priv.h"
#include "fluid_list.h"
#include "fluid_sfont.h"

typedef struct _fluid_gen_info_t {
    char num;        /* Generator number */
    char init;       /* Does the generator need to be initialized (cfr.
                        fluid_voice_init()) */
    char nrpn_scale; /* The scale to convert from NRPN (cfr.
                        fluid_gen_map_nrpn()) */
    float min;       /* The minimum value */
    float max;       /* The maximum value */
    float def; /* The default value (cfr. fluid_gen_set_default_values()) */
} fluid_gen_info_t;

/* See SFSpec21 $8.1.3 */
const static fluid_gen_info_t fluid_gen_info[] = {
    /* number/name             init  scale         min        max         def */
    {GEN_STARTADDROFS, 1, 1, 0.0f, 1e10f, 0.0f},
    {GEN_ENDADDROFS, 1, 1, -1e10f, 0.0f, 0.0f},
    {GEN_STARTLOOPADDROFS, 1, 1, -1e10f, 1e10f, 0.0f},
    {GEN_ENDLOOPADDROFS, 1, 1, -1e10f, 1e10f, 0.0f},
    {GEN_STARTADDRCOARSEOFS, 0, 1, 0.0f, 1e10f, 0.0f},
    {GEN_MODLFOTOPITCH, 1, 2, -12000.0f, 12000.0f, 0.0f},
    {GEN_VIBLFOTOPITCH, 1, 2, -12000.0f, 12000.0f, 0.0f},
    {GEN_MODENVTOPITCH, 1, 2, -12000.0f, 12000.0f, 0.0f},
    {GEN_FILTERFC, 1, 2, 1500.0f, 13500.0f, 13500.0f},
    {GEN_FILTERQ, 1, 1, 0.0f, 960.0f, 0.0f},
    {GEN_MODLFOTOFILTERFC, 1, 2, -12000.0f, 12000.0f, 0.0f},
    {GEN_MODENVTOFILTERFC, 1, 2, -12000.0f, 12000.0f, 0.0f},
    {GEN_ENDADDRCOARSEOFS, 0, 1, -1e10f, 0.0f, 0.0f},
    {GEN_MODLFOTOVOL, 1, 1, -960.0f, 960.0f, 0.0f},
    {GEN_UNUSED1, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_CHORUSSEND, 1, 1, 0.0f, 1000.0f, 0.0f},
    {GEN_REVERBSEND, 1, 1, 0.0f, 1000.0f, 0.0f},
    {GEN_PAN, 1, 1, -500.0f, 500.0f, 0.0f},
    {GEN_UNUSED2, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_UNUSED3, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_UNUSED4, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_MODLFODELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f},
    {GEN_MODLFOFREQ, 1, 4, -16000.0f, 4500.0f, 0.0f},
    {GEN_VIBLFODELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f},
    {GEN_VIBLFOFREQ, 1, 4, -16000.0f, 4500.0f, 0.0f},
    {GEN_MODENVDELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f},
    {GEN_MODENVATTACK, 1, 2, -12000.0f, 8000.0f, -12000.0f},
    {GEN_MODENVHOLD, 1, 2, -12000.0f, 5000.0f, -12000.0f},
    {GEN_MODENVDECAY, 1, 2, -12000.0f, 8000.0f, -12000.0f},
    {GEN_MODENVSUSTAIN, 0, 1, 0.0f, 1000.0f, 0.0f},
    {GEN_MODENVRELEASE, 1, 2, -12000.0f, 8000.0f, -12000.0f},
    {GEN_KEYTOMODENVHOLD, 0, 1, -1200.0f, 1200.0f, 0.0f},
    {GEN_KEYTOMODENVDECAY, 0, 1, -1200.0f, 1200.0f, 0.0f},
    {GEN_VOLENVDELAY, 1, 2, -12000.0f, 5000.0f, -12000.0f},
    {GEN_VOLENVATTACK, 1, 2, -12000.0f, 8000.0f, -12000.0f},
    {GEN_VOLENVHOLD, 1, 2, -12000.0f, 5000.0f, -12000.0f},
    {GEN_VOLENVDECAY, 1, 2, -12000.0f, 8000.0f, -12000.0f},
    {GEN_VOLENVSUSTAIN, 0, 1, 0.0f, 1440.0f, 0.0f},
    {GEN_VOLENVRELEASE, 1, 2, -12000.0f, 8000.0f, -12000.0f},
    {GEN_KEYTOVOLENVHOLD, 0, 1, -1200.0f, 1200.0f, 0.0f},
    {GEN_KEYTOVOLENVDECAY, 0, 1, -1200.0f, 1200.0f, 0.0f},
    {GEN_INSTRUMENT, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_RESERVED1, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_KEYRANGE, 0, 0, 0.0f, 127.0f, 0.0f},
    {GEN_VELRANGE, 0, 0, 0.0f, 127.0f, 0.0f},
    {GEN_STARTLOOPADDRCOARSEOFS, 0, 1, -1e10f, 1e10f, 0.0f},
    {GEN_KEYNUM, 1, 0, 0.0f, 127.0f, -1.0f},
    {GEN_VELOCITY, 1, 1, 0.0f, 127.0f, -1.0f},
    {GEN_ATTENUATION, 1, 1, 0.0f, 1440.0f, 0.0f},
    {GEN_RESERVED2, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_ENDLOOPADDRCOARSEOFS, 0, 1, -1e10f, 1e10f, 0.0f},
    {GEN_COARSETUNE, 0, 1, -120.0f, 120.0f, 0.0f},
    {GEN_FINETUNE, 0, 1, -99.0f, 99.0f, 0.0f},
    {GEN_SAMPLEID, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_SAMPLEMODE, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_RESERVED3, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_SCALETUNE, 0, 1, 0.0f, 1200.0f, 100.0f},
    {GEN_EXCLUSIVECLASS, 0, 0, 0.0f, 0.0f, 0.0f},
    {GEN_OVERRIDEROOTKEY, 1, 0, 0.0f, 127.0f, -1.0f},
    {GEN_PITCH, 1, 0, 0.0f, 127.0f, 0.0f}};


#define fluid_gen_set_mod(_gen, _val)                                          \
    { (_gen)->mod = (double)(_val); }
#define fluid_gen_set_nrpn(_gen, _val)                                         \
    { (_gen)->nrpn = (double)(_val); }

fluid_real_t fluid_gen_scale(int gen, float value);
fluid_real_t fluid_gen_scale_nrpn(int gen, int nrpn);
int fluid_gen_init(fluid_gen_t *gen, fluid_channel_t *channel);


fluid_sf_gen_t * fluid_sf_gen_create(SFGen *sfgen);
fluid_sf_gen_t *fluid_sf_gen_get(fluid_list_t *gen_list, uint8_t num);

void fluid_sf_gen_delete(fluid_sf_gen_t *gen);

#endif /* _FLUID_GEN_H */
