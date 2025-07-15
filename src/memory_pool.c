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

#ifdef __linux__
    #include "fluid_list.h"
    typedef struct {
        void* ptr;       // 分配的内存地址
        size_t size;     // 用户请求的分配大小
    } AllocRecord;

    static AllocRecord alloc_list[409600] = {0}; //静态分配，保证AllocRecord不依赖内存分配器，否则容易产生循环依赖
    static int alloc_index = 0;
#endif

static size_t next_free = 0;

void* simple_malloc(size_t size) {
    #if defined(__arm__) && defined(STM32F407xx)
    if(pool_size == 0) {
        pool_size = 64*1024 - (&_eccmram - &_sccmram);
    }
    #endif

    if (next_free + size > pool_size) {
        FLUID_LOG(FLUID_WARN, "Memory pool overflow(%d): %d > %d\n", size, next_free + size, pool_size);
        return malloc(size);
    }
    
    void* ptr = &memory_pool[next_free];
    
    // 移动空闲指针（简单的对齐到4字节边界）
    size_t aligned_size = (size + 3) & ~3;
    next_free += aligned_size;

    #ifdef __linux__
        alloc_list[alloc_index].ptr = ptr;
        alloc_list[alloc_index].size = size;
        alloc_index += 1;
    #endif

    return ptr;
}


void no_free(void* ptr) {
    (void)ptr;

    #ifdef __linux__
        for(int i=0; i<alloc_index; i++){
            if (alloc_list[i].ptr == ptr) {
                printf("Releasing allocation at %p of size %zu bytes\n", 
                    ptr, alloc_list[i].size);
                return;
            }
        }
        printf("Warning: Attempted to free unknown pointer %p\n", ptr);
    #endif
}

size_t memory_pool_used(){
    return next_free;
}

uint8_t *memory_pool_base(){
    return memory_pool;
}
