#ifndef _FLUIDSYNTH_LOG_H
#define _FLUIDSYNTH_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * FluidSynth log levels.
 */
enum fluid_log_level {
    FLUID_PANIC = 0, /**< The synth can't function correctly any more */
    FLUID_ERR,       /**< Serious error occurred */
    FLUID_WARN,      /**< Warning */
    FLUID_INFO,      /**< Verbose informational messages */
    FLUID_DBG,       /**< Debugging messages */
    LAST_LOG_LEVEL
};

enum fluid_log_level get_log_level(void);
void set_log_level(enum fluid_log_level level);

int fluid_log(enum fluid_log_level level, char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_LOG_H */
