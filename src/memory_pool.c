#include <stdlib.h>
#include "memory_pool.h"
#include "fluidliter.h"

#if defined(__arm__) && defined(STM32F407xx)
    extern uint8_t _sccmram, _eccmram;
    static uint8_t *memory_pool = &_eccmram;
    static size_t pool_size = 0;
#else
    #define MEMORY_POOL_SIZE 64*1024*1024
    static uint8_t memory_pool[MEMORY_POOL_SIZE];
    static size_t pool_size = MEMORY_POOL_SIZE;
#endif


static size_t next_free = 0;

void* simple_malloc(size_t size) {
    #if defined(__arm__) && defined(STM32F407xx)
    if(pool_size == 0) {
        pool_size = 64*1024 - (&_eccmram - &_sccmram);
    }
    #endif

    if (next_free + size > pool_size) {
        FLUID_LOG(FLUID_WARN, "Memory pool overflow: %d > %d\n", next_free + size, pool_size);
        return malloc(size);
    }
    
    void* ptr = &memory_pool[next_free];
    
    // 移动空闲指针（简单的对齐到4字节边界）
    size_t aligned_size = (size + 3) & ~3;
    next_free += aligned_size;
    
    return ptr;
}


void no_free(void* ptr) {
    (void)ptr;
}

size_t memory_pool_used(){
    return next_free;
}

uint8_t *memory_pool_base(){
    return memory_pool;
}
