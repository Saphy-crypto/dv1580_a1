#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Global Variables
static char *memory_pool = NULL;
static bool *allocation_map = NULL;
static size_t total_allocated_memory = 0;  // Track total allocated memory
static size_t pool_size = 0;               // Dynamic pool size

// Initialize Memory Pool
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

    pool_size = size;               // Set the dynamic pool size
    total_allocated_memory = 0;    // Reset allocated memory

    printf("Memory pool of size %zu bytes initialized.\n", size);
}

// Allocate Memory
void* mem_alloc(size_t size) {
    size_t free_blocks = 0;
    size_t start_index = 0;

    // Check if memory pool is initialized
    if (memory_pool == NULL || allocation_map == NULL) {
        printf("Memory pool is not initialized.\n");
        return NULL;
    }

    // Check if requested allocation exceeds the remaining available memory
    if (total_allocated_memory + size > pool_size) {
        printf("Not enough memory available to allocate %zu bytes. Total allocated: %zu bytes.\n", size, total_allocated_memory);
        return NULL;
    }

    // First-fit strategy: find the first contiguous free block big enough
    for (size_t i = 0; i < pool_size; i++) {
        if (!allocation_map[i]) {
            if (free_blocks == 0) {
                start_index = i;
            }
            free_blocks++;

            if (free_blocks == size) {
                // Mark these blocks as allocated
                for (size_t j = start_index; j < start_index + size; j++) {
                    allocation_map[j] = true;
                }
                total_allocated_memory += size;
                printf("Allocated %zu bytes at index %zu. Total allocated: %zu bytes.\n", size, start_index, total_allocated_memory);
                return memory_pool + start_index;
            }
        } else {
            free_blocks = 0;
        }
    }

    printf("Not enough contiguous memory available to allocate %zu bytes.\n", size);
    return NULL;
}

// Free Memory
void mem_free(void* block) {
    if (block == NULL || (char*)block < memory_pool || (char*)block >= memory_pool + pool_size) {
        printf("Invalid block pointer. It does not belong to the memory pool.\n");
        return;
    }
    size_t start_index = (char*)block - memory_pool;
    size_t freed_blocks = 0;

    // Free contiguous allocated blocks starting from the block
    while (start_index < pool_size && allocation_map[start_index]) {
        allocation_map[start_index] = false;
        start_index++;
        freed_blocks++;
    }

    if (freed_blocks > total_allocated_memory) {
        printf("Error: Freed blocks exceed total allocated memory.\n");
        total_allocated_memory = 0;
    } else {
        total_allocated_memory -= freed_blocks;
    }

    printf("Memory block freed. Freed %zu bytes. Total allocated: %zu bytes.\n", freed_blocks, total_allocated_memory);
}

// Resize Memory
void* mem_resize(void* block, size_t new_size) {
    if (block == NULL) {
        return mem_alloc(new_size);
    }

    size_t start_index = (char*)block - memory_pool;
    size_t current_size = 0;

    // Determine the current size of the block
    while (start_index + current_size < pool_size && allocation_map[start_index + current_size]) {
        current_size++;
    }

    // If the new size is smaller or equal, free the extra space
    if (new_size <= current_size) {
        for (size_t i = start_index + new_size; i < start_index + current_size; i++) {
            allocation_map[i] = false;
        }
        total_allocated_memory -= (current_size - new_size);
        printf("Resized block at index %zu to %zu bytes. Total allocated: %zu bytes.\n", start_index, new_size, total_allocated_memory);
        return block;
    }

    // Check if the block can be expanded in place
    size_t i;
    for (i = start_index + current_size; i < start_index + new_size; i++) {
        if (i >= pool_size || allocation_map[i]) {
            break;
        }
    }

    if (i == start_index + new_size) {
        // Expand the block in place
        for (size_t j = start_index + current_size; j < start_index + new_size; j++) {
            allocation_map[j] = true;
        }
        total_allocated_memory += (new_size - current_size);
        printf("Expanded block at index %zu to %zu bytes. Total allocated: %zu bytes.\n", start_index, new_size, total_allocated_memory);
        return block;
    }

    // Allocate a new block and move the data
    void* new_block = mem_alloc(new_size);
    if (new_block) {
        memcpy(new_block, block, current_size);
        mem_free(block);
        printf("Resized block by allocating new block of %zu bytes and freeing old block. Total allocated: %zu bytes.\n", new_size, total_allocated_memory);
    }

    return new_block;
}

// Deinitialize Memory Pool
void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);
        memory_pool = NULL;
    }

    if (allocation_map != NULL) {
        free(allocation_map);
        allocation_map = NULL;
    }

    total_allocated_memory = 0;
    pool_size = 0;

    printf("Memory pool deinitialized.\n");
}

// Print Allocation Map (for debugging)
void print_allocation_map() {
    printf("Allocation Map: ");
    for (size_t i = 0; i < pool_size; i++) {
        printf("%d", allocation_map[i]);
    }
    printf("\n");
}
