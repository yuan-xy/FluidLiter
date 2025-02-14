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


#include "fluidliter/types.h"
#include "fluidliter/synth.h"
#include "fluidliter/sfont.h"
#include "fluidliter/log.h"
#include "fluidliter/mod.h"
#include "fluidliter/gen.h"
#include "fluidliter/voice.h"
#include "fluidliter/music.h"

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDLITE_H */
