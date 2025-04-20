#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <assert.h>
#include <stdbool.h>
#include "fluidliter.h"


// 第一层：将参数转换为字符串
#define STRINGIFY(x) #x
// 第二层：展开参数，再调用STRINGIFY
#define TOSTRING(x) STRINGIFY(x)


int main(int argc, char *argv[])
{
    //测试编译器内置的宏
    #if defined(__x86_64__)
        printf("CPU: x86_64 (Intel/AMD 64-bit)\n");
    #elif defined(__i386__)
        printf("CPU: x86 (32-bit)\n");
    #elif defined(__aarch64__)
        printf("CPU: ARM64 (AArch64)\n");
    #elif defined(__arm__)
        printf("CPU: ARM (32-bit)\n");
    #else
        printf("Unknown CPU\n");
    #endif


    //测试makefile里自定义的宏
    #ifdef ARCH
        printf("ARCH = %s\n", TOSTRING(ARCH));
        #if ARCH == i386
            printf("ARCH is i386\n");  //直接==比较是成功的，但是TOSTRING(ARCH)的输出却永远是1？ 
        #else
            printf("ARCH is NOT i386\n");
        #endif
    #else
        printf("ARCH is not defined\n");
    #endif

    #ifdef TEST
        printf("TEST = %s\n", STRINGIFY(TEST));
        printf("TEST = %s\n", TOSTRING(TEST));
        #if TEST == test2
            printf("Pass compare of macro TEST\n");
        #endif
    #else
        printf("TEST is not defined\n");
    #endif    
    
    return 0;
}

// make clean
// make ARCH=x86_64
// make ARCH=x86_64  test_macro_run

// CPU: x86_64 (Intel/AMD 64-bit)
// ARCH = x86_64
// ARCH is i386
// TEST = TEST
// TEST = test2
// Pass compare of macro TEST



// make clean
// make
// make test_macro_run

// CPU: x86 (32-bit)
// ARCH = 1
// ARCH is i386
// TEST = TEST
// TEST = test2
// Pass compare of macro TEST