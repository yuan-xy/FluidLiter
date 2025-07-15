#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

#include <stddef.h>
#include <stdint.h>

void* simple_malloc(size_t size);

void no_free(void* ptr);

size_t memory_pool_used();

#endif /* _MEMORY_POOL_H */

