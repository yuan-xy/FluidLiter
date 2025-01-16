#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

#include "fluidsynth_priv.h"


void fluid_sys_config(void);
void fluid_log_config(void);


/*
 * Utility functions
 */
char *fluid_strtok (char **str, char *delim);



/**
    Profile numbers. List all the pieces of code you want to profile
    here. Be sure to add an entry in the fluid_profile_data table in
    fluid_sys.c
*/
enum {
  FLUID_PROF_WRITE_S16,
  FLUID_PROF_ONE_BLOCK,
  FLUID_PROF_ONE_BLOCK_CLEAR,
  FLUID_PROF_ONE_BLOCK_VOICE,
  FLUID_PROF_ONE_BLOCK_VOICES,
  FLUID_PROF_ONE_BLOCK_REVERB,
  FLUID_PROF_ONE_BLOCK_CHORUS,
  FLUID_PROF_VOICE_NOTE,
  FLUID_PROF_VOICE_RELEASE,
  FLUID_PROF_LAST
};


/* Profiling */


/**

    Memory locking

    Memory locking is used to avoid swapping of the large block of
    sample data.
 */


/**

    Floating point exceptions

    fluid_check_fpe() checks for "unnormalized numbers" and other
    exceptions of the floating point processsor.
*/


#endif /* _FLUID_SYS_H */
