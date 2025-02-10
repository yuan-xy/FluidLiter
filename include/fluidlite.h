#ifndef _FLUIDLITE_H
#define _FLUIDLITE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#if defined(FLUIDLITE_STATIC)
#define FLUIDSYNTH_API
#elif defined(FLUIDLITE_DLL_EXPORTS)
#define FLUIDSYNTH_API __declspec(dllexport)
#else
#define FLUIDSYNTH_API __declspec(dllimport)
#endif
#elif (defined(__GNUC__) || defined(__clang__))
#if defined(FLUIDLITE_STATIC)
#define FLUIDSYNTH_API
#else
#define FLUIDSYNTH_API __attribute__((visibility("default")))
#endif
#elif defined(__OS2__) && defined(__WATCOMC__)
#if defined(FLUIDLITE_STATIC)
#define FLUIDSYNTH_API
#elif defined(FLUIDLITE_DLL_EXPORTS)
#define FLUIDSYNTH_API __declspec(dllexport)
#else
#define FLUIDSYNTH_API
#endif
#endif

#include "fluid_config.h"

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
