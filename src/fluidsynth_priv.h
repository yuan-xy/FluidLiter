
#ifndef _FLUIDLITE_PRIV_H
#define _FLUIDLITE_PRIV_H

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



typedef enum { FLUID_OK = 0, FLUID_FAILED = -1 } fluid_status;


#if defined(__LP64__) || defined(_WIN64)
typedef uint64_t uintptr;
#else
typedef uint32_t uintptr;
#endif


/***************************************************************
 *
 *       FORWARD DECLARATIONS
 */
typedef struct _fluid_env_data_t fluid_env_data_t;
typedef struct _fluid_channel_t fluid_channel_t;
typedef struct _fluid_tuning_t fluid_tuning_t;
typedef struct _fluid_hashtable_t fluid_hashtable_t;

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

#define FLUID_REALLOC(_p, _n) realloc(_p, _n)

#ifdef USING_CALLOC
    #define FLUID_MALLOC(_n) calloc(1, _n)
    #define FLUID_NEW(_t) (_t *)calloc(1, sizeof(_t))
    #define FLUID_ARRAY(_t, _n) (_t *)calloc((_n) , sizeof(_t))
#else
    #define FLUID_MALLOC(_n) malloc(_n)
    #define FLUID_NEW(_t) (_t *)malloc(sizeof(_t))
    #define FLUID_ARRAY(_t, _n) (_t *)malloc((_n) * sizeof(_t))

#endif
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
#define FLUID_STRCPY(_dst, _src) snprintf(_dst, sizeof(_dst), "%s", _src)
#define FLUID_STRCHR(_s, _c) strchr(_s, _c)
#define FLUID_STRDUP(s)                                                        \
    strcpy((char *)calloc(FLUID_STRLEN(s) + 1), s)
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

#if DEBUG
#define FLUID_LOG fluid_log
#else
#define FLUID_LOG(...) (void)0;
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530941723212145818
#endif

#ifndef M_LN10
#define M_LN10 2.3025850929940456840179914546844
#endif

#define FLUID_M_PI      ((fluid_real_t)M_PI)
#define FLUID_M_LN2     ((fluid_real_t)M_LN2)
#define FLUID_M_LN10    ((fluid_real_t)M_LN10)



/* Math functions */
#if defined WITH_FLOAT
#define FLUID_SIN   sinf
#else
#define FLUID_SIN   (fluid_real_t)sin
#endif


#if defined WITH_FLOAT
#define FLUID_COS   cosf
#else
#define FLUID_COS   (fluid_real_t)cos
#endif

#if defined WITH_FLOAT
#define FLUID_FABS  fabsf
#else
#define FLUID_FABS  (fluid_real_t)fabs
#endif

#if defined WITH_FLOAT
#define FLUID_POW   powf
#else
#define FLUID_POW   (fluid_real_t)pow
#endif

#if defined WITH_FLOAT
#define FLUID_SQRT  sqrtf
#else
#define FLUID_SQRT  (fluid_real_t)sqrt
#endif

#if defined WITH_FLOAT
#define FLUID_LOGF  logf
#else
#define FLUID_LOGF  (fluid_real_t)log
#endif


/* Internationalization */
#define _(s) s

#endif /* _FLUIDLITE_PRIV_H */
