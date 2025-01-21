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

#include <execinfo.h>  // 用于 backtrace
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
    if(count%10000==9999){
        printf("%d\n", count);
        //print_stack_trace();  // 打印调用栈
    }
    if(count>1000000) exit(0);
}


#define MIRCO_SECOND 1000000
#define SAMPLE_RATE 44100
#define NUM_FRAMES 441                    // SAMPLE_RATE*DURATION
#define DURATION (NUM_FRAMES / SAMPLE_RATE) // second
#define NUM_CHANNELS 1
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)




int main(int argc, char *argv[])
{
    fluid_synth_t *synth = NEW_FLUID_SYNTH(.verbose=0, .polyphony=1);
    int sfont = fluid_synth_sfload(synth, "example/sf_/GMGSx_1.sf2", 1);
    fluid_synth_program_select(synth, 0, sfont, 0, 0);

    int16_t *buffer = calloc(sizeof(int16_t) , NUM_SAMPLES);

    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_INEXACT);
    signal(SIGFPE, handle_fpe); 

    fluid_synth_noteon(synth, 0, C, 100);
    fluid_synth_write_s16_mono(synth, NUM_FRAMES, buffer);

    printf("FE_INEXACT occured %d times.\n ", count);

    free(buffer);
    delete_fluid_synth(synth);
    return 0;
}

