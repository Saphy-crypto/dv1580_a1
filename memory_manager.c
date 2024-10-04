#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define POOL_SIZE 2048  // Define the size of the memory pool

// Pointer to the beginning of the memory pool
static char *memory_pool = NULL;
// Array to track which parts of the pool are allocated
static bool *allocation_map = NULL;
static size_t total_allocated_memory = 0;  // Track total allocated memory

void mem_init(size_t size) {
    if (size == 0) {
        printf("Size must be greater than zero.\n");
        exit(1);
    }

    // Initialize the memory pool and allocation map
    memory_pool = (char*)malloc(size);
    if (memory_pool == NULL) {
        printf("Memory pool allocation failed!\n");
        exit(1);
    }

    allocation_map = (bool*)malloc(size * sizeof(bool));
    if (allocation_map == NULL) {
        printf("Allocation map creation failed!\n");
        free(memory_pool);
        exit(1);
    }

    // Initialize the allocation map to false (all memory is free)
    for (size_t i = 0; i < size; i++) {
        allocation_map[i] = false;
    }

    printf("Memory pool of size %zu bytes initialized.\n", size);
}

void* mem_alloc(size_t size) {
    size_t best_fit_index = -1;
    size_t best_fit_size = POOL_SIZE;

    // Check if memory pool is initialized
    if (memory_pool == NULL || allocation_map == NULL) {
        printf("Memory pool is not initialized.\n");
        return NULL;
    }

    // Calculate the total allocated memory
    size_t total_allocated = 0;
    for (size_t i = 0; i < POOL_SIZE; i++) {
        if (allocation_map[i]) {
            total_allocated++;
        }
    }

    // Check if the allocation would exceed the total available memory
    if (total_allocated + size > POOL_SIZE) {
        printf("Cumulative allocations exceed total available memory.\n");
        return NULL;
    }

    // Iterate through the allocation map to find the best fit
    for (size_t i = 0; i < POOL_SIZE; i++) {
        if (!allocation_map[i]) {
            // Start counting free blocks
            size_t free_blocks = 1;
            size_t start_index = i;

            // Count contiguous free blocks
            while (i + free_blocks < POOL_SIZE && !allocation_map[i + free_blocks]) {
                free_blocks++;
            }

            // Check if this block is a better fit than the current best fit
            if (free_blocks >= size && free_blocks < best_fit_size) {
                best_fit_index = start_index;
                best_fit_size = free_blocks;
            }
        }
    }

    // If a best fit was found, allocate the memory
    if (best_fit_index != -1) {
        // Mark these blocks as allocated
        for (size_t j = best_fit_index; j < best_fit_index + size; j++) {
            allocation_map[j] = true;
        }
        return memory_pool + best_fit_index;
    }

    // If no best fit was found, return NULL
    printf("Not enough memory available to allocate %zu bytes.\n", size);
    return NULL;
}

void mem_free(void* block) {
    if (block == NULL || (char*)block < memory_pool || (char*)block >= memory_pool + POOL_SIZE) {
        printf("Invalid block pointer. It does not belong to the memory pool.\n");
        return;
    }
    size_t start_index = (char*)block - memory_pool;
    size_t freed_blocks = 0;

    // Free contiguous allocated blocks starting from the block
    while (start_index < POOL_SIZE && allocation_map[start_index]) {
        allocation_map[start_index] = false;
        start_index++;
        freed_blocks++;
    }

    total_allocated_memory -= freed_blocks;
    printf("Memory block freed.\n");
}

void* mem_resize(void* block, size_t new_size) {
    if (block == NULL) {
        return mem_alloc(new_size);
    }

    size_t start_index = (char*)block - memory_pool;
    size_t current_size = 0;

    // Determine the current size of the block
    while (start_index + current_size < POOL_SIZE && allocation_map[start_index + current_size]) {
        current_size++;
    }

    // If the new size is smaller or equal, free the extra space
    if (new_size <= current_size) {
        for (size_t i = start_index + new_size; i < start_index + current_size; i++) {
            allocation_map[i] = false;
        }
        return block;
    }

    // Check if the block can be expanded in place
    size_t i;
    for (i = start_index + current_size; i < start_index + new_size; i++) {
        if (i >= POOL_SIZE || allocation_map[i]) {
            break;
        }
    }

    if (i == start_index + new_size) {
        // Expand the block in place
        for (size_t j = start_index + current_size; j < start_index + new_size; j++) {
            allocation_map[j] = true;
        }
        return block;
    }

    // Allocate a new block and move the data
    void* new_block = mem_alloc(new_size);
    if (new_block) {
        memcpy(new_block, block, current_size);
        mem_free(block);
    }

    return new_block;
}

void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);
        memory_pool = NULL;
    }

    if (allocation_map != NULL) {
        free(allocation_map);
        allocation_map = NULL;
    }

    printf("Memory pool deinitialized.\n");
}

void print_allocation_map() {
    printf("Allocation Map: ");
    for (size_t i = 0; i < POOL_SIZE; i++) {
        printf("%d", allocation_map[i]);
    }
    printf("\n");
}