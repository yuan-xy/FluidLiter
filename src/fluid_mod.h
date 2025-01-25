#ifndef _FLUID_MOD_H
#define _FLUID_MOD_H

#include "fluidsynth_priv.h"
#include "fluid_conv.h"

void fluid_mod_clone(fluid_mod_t *mod, fluid_mod_t *src);
fluid_real_t fluid_mod_get_value(fluid_mod_t *mod, fluid_channel_t *chan,
                                 fluid_voice_t *voice);
void fluid_dump_modulator(fluid_mod_t *mod);

#define fluid_mod_has_source(mod, cc, ctrl)                                    \
    (((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) != 0) &&       \
       (cc != 0)) ||                                                           \
      ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) == 0) &&      \
        (cc == 0)))) ||                                                        \
     ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) != 0) &&       \
       (cc != 0)) ||                                                           \
      ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) == 0) &&      \
        (cc == 0)))))

#define fluid_mod_has_dest(mod, gen) ((mod)->dest == gen)

#endif /* _FLUID_MOD_H */
