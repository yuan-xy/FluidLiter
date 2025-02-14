#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidlite.h"
#include "fluid_synth.h"
#include "misc.h"

#include <float.h> // 用于定义 FLT_MIN

#define __USE_GNU
#include <fenv.h>

#ifndef __ARM_EABI__
#include <execinfo.h>  // 用于 backtrace
#endif

#include <signal.h>    // 用于信号处理
// #include <unistd.h>    // 用于 getpid

void print_stack_trace() {
    void *buffer[100];  // 存储调用栈地址
    int size = backtrace(buffer, 100);  // 获取调用栈
    char **strings = backtrace_symbols(buffer, size);  // 将地址转换为可读的字符串

    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    printf("Call stack:\n");
    for (int i = 0; i < size; i++) {
        printf("%s\n", strings[i]);  // 打印每一层的调用信息
    }

    free(strings);  // 释放内存
}

static int count=0;

void handle_fpe(int sig) {
    count+=1;
    if(count%100000==99999){
        //printf("%d\n", count);
        //print_stack_trace();  // 打印调用栈
    }
    if(count>1000000) exit(0); //如果不主动exit，计数会停留在2147479999
}


#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 441                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)



extern int roundi (float x);

void test_float(){
#ifdef WITH_FLOAT
    assert(sizeof(float) == sizeof(fluid_real_t));
#endif
    assert( 25 == (float)((33875-98481)*(uint32_t)1000/168000000) );
    assert( 25564 == (float)((33875-98481)*(uint32_t)1/168000) );
    assert( float_eq(25564.8984,  ((uint32_t)-64605*(float)1.0/168000)) );
    assert( float_eq(-0.384553581, (float)((int32_t)-64605*(float)1.0/168000) ));

    assert(sizeof(short)==sizeof(int16_t));
    assert((int)0.5f == 0);
    assert((int)-0.5f == 0);
    assert((int)0.9999f == 0);
    assert((int)1.0f == 1);
}



int main(int argc, char *argv[])
{
#ifdef __ARM_EABI__
    return 0;
#else
    test_float();
    fluid_synth_t *synth = NEW_FLUID_SYNTH(.polyphony=1);
    set_log_level(FLUID_WARN);
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_INEXACT);
    signal(SIGFPE, handle_fpe); 

    fluid_synth_noteon(synth, 0, NOTE_C, 100);
    fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);

    printf("FE_INEXACT occured %d times.\n ", count);

    free(buffer);
    delete_fluid_synth(synth);
    return 0;
#endif
}

