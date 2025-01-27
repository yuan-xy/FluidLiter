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

/**
 * @file fluidlite.h
 * @brief FluidLite is a real-time synthesizer designed for SoundFont(R) files.
 *
 * This is the header of the fluidlite library and contains the
 * synthesizer's public API.
 *
 * Depending on how you want to use or extend the synthesizer you
 * will need different API functions. You probably do not need all
 * of them. Here is what you might want to do:
 *
 * o Embedded synthesizer: create a new synthesizer and send MIDI
 *   events to it. The sound goes directly to the audio output of
 *   your system.
 *
 * o Plugin synthesizer: create a synthesizer and send MIDI events
 *   but pull the audio back into your application.
 *
 * o SoundFont plugin: create a new type of "SoundFont" and allow
 *   the synthesizer to load your type of SoundFonts.
 *
 * o MIDI input: Create a MIDI handler to read the MIDI input on your
 *   machine and send the MIDI events directly to the synthesizer.
 *
 * o MIDI files: Open MIDI files and send the MIDI events to the
 *   synthesizer.
 *
 * o Command lines: You can send textual commands to the synthesizer.
 *
 * SoundFont(R) is a registered trademark of E-mu Systems, Inc.
 */

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
