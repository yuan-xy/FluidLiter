#ifndef _FLUIDLITE_H
#define _FLUIDLITE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif


#include "fluidlite/types.h"
#include "fluidlite/synth.h"
#include "fluidlite/sfont.h"
#include "fluidlite/log.h"
#include "fluidlite/mod.h"
#include "fluidlite/gen.h"
#include "fluidlite/voice.h"
#include "fluidlite/music.h"

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDLITE_H */
