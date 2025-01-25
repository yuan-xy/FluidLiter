#ifndef _FLUID_CONV_H
#define _FLUID_CONV_H

#include "fluidsynth_priv.h"

#define FLUID_CENTS_HZ_SIZE 1200
#define FLUID_VEL_CB_SIZE 128
#define FLUID_CB_AMP_SIZE 1441
#define FLUID_PAN_SIZE 1002

fluid_real_t fluid_ct2hz_real(fluid_real_t cents);
fluid_real_t fluid_ct2hz(fluid_real_t cents);
fluid_real_t fluid_cb2amp(fluid_real_t cb);
fluid_real_t fluid_tc2sec(fluid_real_t tc);
fluid_real_t fluid_tc2sec_delay(fluid_real_t tc);
fluid_real_t fluid_tc2sec_attack(fluid_real_t tc);
fluid_real_t fluid_tc2sec_release(fluid_real_t tc);
fluid_real_t fluid_act2hz(fluid_real_t c);
fluid_real_t fluid_hz2ct(fluid_real_t c);
fluid_real_t fluid_pan(fluid_real_t c, int left);
fluid_real_t fluid_concave(fluid_real_t val);
fluid_real_t fluid_convex(fluid_real_t val);

#endif /* _FLUID_CONV_H */
