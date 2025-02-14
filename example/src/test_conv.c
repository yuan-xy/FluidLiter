#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"
#include "fluid_synth.h"
#include "misc.h"

#include <time.h>
#include <stdint.h>

// 读取时间戳计数器
static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}


void task(){
    for(int i=0;i<100;i++) fluid_ct2hz(7000.0+i);
}

int main(int argc, char *argv[])
{
    // echo | arm-none-eabi-gcc -dM -E -
    #ifdef __linux__
    {
        uint64_t start, end;
        start = rdtsc();
        task();
        end = rdtsc();
        printf("Cycles used: %llu\n", end - start);
    };    
    {
        struct timespec start, end;
        long time_used;

        clock_gettime(CLOCK_MONOTONIC, &start);
        task();
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_used = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
        printf("CPU time used: %ld nano seconds\n", time_used);
    };
    #endif

    {
        clock_t start, end;
        long cpu_time_used;

        start = clock();
        task();
        end = clock();
        cpu_time_used = ((double) (end - start));
        printf("CPU time used: %ld/%ld seconds\n", cpu_time_used, CLOCKS_PER_SEC);
    }


    assert(float_eq(fluid_act2hz(6000.0), 261.62557));
    assert(float_eq(fluid_act2hz(6900.0), 440));

    assert(float_eq(fluid_act2hz(1500.0), 19.445));
    assert(float_eq(fluid_act2hz(13500), 19912.127));

    assert(float_eq(fluid_ct2hz(1500.0), 19.445));
    assert(float_eq(fluid_ct2hz(13500), 19912.127));

    for(int i = 1500; i < 13500; i++){
        assert(float_eq(fluid_ct2hz_real(i), fluid_act2hz(i)));
    }

    return 0;
}

