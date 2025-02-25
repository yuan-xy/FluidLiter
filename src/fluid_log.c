#include "fluidsynth_priv.h"

#ifndef ERR_BUF_LEN
#define ERR_BUF_LEN 128
#endif

#if DEBUG
static char fluid_errbuf[ERR_BUF_LEN]; /* buffer for error message */
#else
static char fluid_errbuf[0]; /* no buffer for error message when release */
#endif


const char *fluid_libname = "FL";

#ifdef DEFAULT_LOG_LEVEL
enum fluid_log_level LOG_LEVEL = DEFAULT_LOG_LEVEL;
#else
enum fluid_log_level LOG_LEVEL = FLUID_INFO;
#endif

enum fluid_log_level get_log_level(void) {
    return LOG_LEVEL;
}

void set_log_level(enum fluid_log_level level) {
    LOG_LEVEL = level;
}

/**
 * Default log function which prints to the stderr.
 * @param level Log level
 * @param message Log message
 */
void fluid_default_log_function(enum fluid_log_level level, char *message) {
    FILE *out;

#if defined(WIN32)
    out = stdout;
#else
    if(level < FLUID_INFO) out = stderr;
    else out = stdout;
#endif

    switch (level) {
    case FLUID_PANIC:
        FLUID_FPRINTF(out, "%s: panic: %s\n", fluid_libname, message);
        break;
    case FLUID_ERR:
        FLUID_FPRINTF(out, "%s: error: %s\n", fluid_libname, message);
        break;
    case FLUID_WARN:
        FLUID_FPRINTF(out, "%s: warning: %s\n", fluid_libname, message);
        break;
    case FLUID_INFO:
        FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
        break;
    case FLUID_DBG:
#if DEBUG
        FLUID_FPRINTF(out, "%s: debug: %s\n", fluid_libname, message);
#endif
        break;
    default:
        FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
        break;
    }
    fflush(out);
}

/**
 * Print a message to the log.
 * @param level Log level (#fluid_log_level).
 * @param fmt Printf style format string for log message
 * @param ... Arguments for printf 'fmt' message string
 * @return return str len or -1
 */
int fluid_log(enum fluid_log_level level, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(fluid_errbuf, sizeof(fluid_errbuf), fmt, args);
    va_end(args);

    if (level <= LOG_LEVEL) {
        fluid_default_log_function(level, fluid_errbuf);
        return ret;
    }
    return FLUID_FAILED;
}
