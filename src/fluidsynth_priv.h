
#ifndef _FLUIDLITE_PRIV_H
#define _FLUIDLITE_PRIV_H

#include "fluid_config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <fcntl.h>
#include <limits.h>

#include <stdint.h>
#include "fluidlite.h"

#define FLUID_INLINE              inline

#define FLUID_N_ELEMENTS(struct)  (sizeof (struct) / sizeof (struct[0]))
#define FLUID_MEMBER_SIZE(struct, member)  ( sizeof (((struct *)0)->member) )

#define fluid_return_if_fail(cond) \
if(cond) \
    ; \
else \
    return

#define fluid_return_val_if_fail(cond, val) \
 fluid_return_if_fail(cond) (val)


/***************************************************************
 *
 *         BASIC TYPES
 */

#if defined(WITH_FLOAT)
typedef float fluid_real_t;
#else
typedef double fluid_real_t;
#endif

typedef enum { FLUID_OK = 0, FLUID_FAILED = -1 } fluid_status;

// socket disabled

/** Integer types  */

#if 1 /**/
typedef signed char sint8;
typedef unsigned char uint8;
typedef signed short sint16;
typedef unsigned short uint16;
typedef signed int sint32;
typedef unsigned int uint32;
typedef int64_t sint64;
typedef uint64_t uint64;

#if defined(__LP64__) || defined(_WIN64)
typedef uint64 uintptr;
#else
typedef uint32 uintptr;
#endif

#else /**/
#if defined(MINGW32)

/* Windows using MinGW32 */
typedef int8_t sint8;
typedef uint8_t uint8;
typedef int16_t sint16;
typedef uint16_t uint16;
typedef int32_t sint32;
typedef uint32_t uint32;
typedef int64_t sint64;
typedef uint64_t uint64;

#elif defined(_WIN32)

/* Windows */
typedef signed __int8 sint8;
typedef unsigned __int8 uint8;
typedef signed __int16 sint16;
typedef unsigned __int16 uint16;
typedef signed __int32 sint32;
typedef unsigned __int32 uint32;
typedef signed __int64 sint64;
typedef unsigned __int64 uint64;

#else

/* Linux & Darwin */
typedef int8_t sint8;
typedef u_int8_t uint8;
typedef int16_t sint16;
typedef u_int16_t uint16;
typedef int32_t sint32;
typedef u_int32_t uint32;
typedef int64_t sint64;
typedef u_int64_t uint64;

#endif
#endif /* */

/***************************************************************
 *
 *       FORWARD DECLARATIONS
 */
typedef struct _fluid_env_data_t fluid_env_data_t;
typedef struct _fluid_adriver_definition_t fluid_adriver_definition_t;
typedef struct _fluid_channel_t fluid_channel_t;
typedef struct _fluid_tuning_t fluid_tuning_t;
typedef struct _fluid_hashtable_t fluid_hashtable_t;
typedef struct _fluid_client_t fluid_client_t;

/***************************************************************
 *
 *                      CONSTANTS
 */

#define FLUID_BUFSIZE 64

#ifndef PI
#define PI 3.141592654
#endif

/***************************************************************
 *
 *                      SYSTEM INTERFACE
 */
typedef FILE *fluid_file;

#define FLUID_MALLOC(_n) malloc(_n)
#define FLUID_REALLOC(_p, _n) realloc(_p, _n)
#define FLUID_NEW(_t) (_t *)malloc(sizeof(_t))
#define FLUID_ARRAY(_t, _n) (_t *)malloc((_n) * sizeof(_t))
#define FLUID_FREE(_p) free(_p)
#define FLUID_FOPEN(_f, _m) fopen(_f, _m)
#define FLUID_FCLOSE(_f) fclose(_f)
#define FLUID_FREAD(_p, _s, _n, _f) fread(_p, _s, _n, _f)
#define FLUID_FSEEK(_f, _n, _set) fseek(_f, _n, _set)
#define FLUID_FTELL(_f) ftell(_f)
#define FLUID_MEMCPY(_dst, _src, _n) memcpy(_dst, _src, _n)
#define FLUID_MEMSET(_s, _c, _n) memset(_s, _c, _n)
#define FLUID_STRLEN(_s) strlen(_s)
#define FLUID_STRCMP(_s, _t) strcmp(_s, _t)
#define FLUID_STRNCMP(_s, _t, _n) strncmp(_s, _t, _n)
#define FLUID_STRCPY(_dst, _src) strcpy(_dst, _src)
#define FLUID_STRCHR(_s, _c) strchr(_s, _c)
#define FLUID_STRDUP(s)                                                        \
    FLUID_STRCPY((char *)FLUID_MALLOC(FLUID_STRLEN(s) + 1), s)
#define FLUID_FPRINTF fprintf

#define fluid_clip(_val, _min, _max)                                           \
    {                                                                          \
        (_val) = ((_val) < (_min)) ? (_min)                                    \
                                   : (((_val) > (_max)) ? (_max) : (_val));    \
    }

#if WITH_FTS
#define FLUID_PRINTF post
#define FLUID_FLUSH()
#else
#define FLUID_PRINTF printf
#define FLUID_FLUSH() fflush(stdout)
#endif

#define FLUID_LOG fluid_log

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#define FLUID_ASSERT(a, b)
#define FLUID_ASSERT_P(a, b)

char *fluid_error(void);

/* Internationalization */
#define _(s) s

#endif /* _FLUIDLITE_PRIV_H */
