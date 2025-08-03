
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
#include "fluidliter.h"
#include "memory_pool.h"


#define fluid_return_if_fail(cond) \
if(cond) \
    ; \
else \
    return

#define fluid_return_val_if_fail(cond, val) \
 fluid_return_if_fail(cond) (val)


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
// typedef struct _fluid_hashtable_t fluid_hashtable_t;

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

// #define SIMPLE_MEM_ALLOC 1

#if SIMPLE_MEM_ALLOC
    #define FLUID_MALLOC(_n) ({ \
    printf("%s line %d, %s:%d\n", __FILE__, __LINE__, #_n, (_n)); \
    simple_malloc(_n); \
    })

    #define FLUID_NEW(_t) ({ \
    printf("%s line %d, %s:%d\n", __FILE__, __LINE__, #_t, sizeof(_t)); \
    (_t *)simple_malloc(sizeof(_t)); \
    })
     
    #define FLUID_ARRAY(_t, _n) ({ \
    printf("%s line %d, %s:%d\n", __FILE__, __LINE__, #_t, (_n) * sizeof(_t)); \
    (_t *)simple_malloc((_n) * sizeof(_t)); \
    })

    #define FLUID_FREE(_p) ({ \
    printf("%s line %d, Free %s:%p\n", __FILE__, __LINE__, #_p, (_p)); \
    no_free(_p); \
    })
#else
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
#endif

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
#define FLUID_STRCHR(_s, _c) strchr(_s, _c)
#define FLUID_STRDUP(s)                                                        \
    strcpy((char *)calloc(1, FLUID_STRLEN(s) + 1), s)

#define FLUID_FPRINTF fprintf

#define fluid_clip(_val, _min, _max)                                           \
    {                                                                          \
        (_val) = ((_val) < (_min)) ? (_min)                                    \
                                   : (((_val) > (_max)) ? (_max) : (_val));    \
    }


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



#if defined WITH_FLOAT
    #define FLUID_SIN   sinf
    #define FLUID_COS   cosf
    #define FLUID_FABS  fabsf
    #define FLUID_POW   powf
    #define FLUID_SQRT  sqrtf
    #define FLUID_LOGF  logf
#else
    #define FLUID_SIN   (fluid_real_t)sin
    #define FLUID_COS   (fluid_real_t)cos
    #define FLUID_FABS  (fluid_real_t)fabs
    #define FLUID_POW   (fluid_real_t)pow
    #define FLUID_SQRT  (fluid_real_t)sqrt
    #define FLUID_LOGF  (fluid_real_t)log
#endif


#if SPI_FLASH == 1
extern  int16_t spi_read_int16(uint32_t base, uint16_t pos);
#define SPI_READ_SAMPLE(base, pos) spi_read_int16(base, pos) 
#endif

#if defined(__arm__) && defined(SPI_READ_SAMPLE) 
    #define READ_SAMPLE(base, pos) \
        ((int)(base) > 0x08000000 ? base[pos] : SPI_READ_SAMPLE((base), (pos)))
#else
    #ifndef READ_SAMPLE
    #define READ_SAMPLE(base, pos) base[pos]
    #endif
#endif


#ifdef G_LIKELY
#define FLUID_LIKELY G_LIKELY
#define FLUID_UNLIKELY G_UNLIKELY
#else
#define FLUID_LIKELY 
#define FLUID_UNLIKELY 
#endif


/* Internationalization */
#define _(s) s

#endif /* _FLUIDLITE_PRIV_H */
