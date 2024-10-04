#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>

// Function declarations for the memory manager

void mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void* block);
void* mem_resize(void* block, size_t new_size);
void mem_deinit();
void print_allocation_map();

#endif // MEMORY_MANAGER_H
