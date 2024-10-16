#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Global Variables
static char *memory_pool = NULL;  // Pointer to the start of the memory pool
static bool *allocation_map = NULL;        // Tracks which bytes are allocated
static size_t *allocation_size_map = NULL;// Records the size of each allocation
static size_t total_allocated_memory = 0;// Keeps track of total allocated memory
static size_t pool_size = 0;       // Total size of the memory pool

/**
 * @brief Initialize the memory pool with a given size.
 *
 * Allocates memory for the pool and initializes allocation maps.
 *
 * @param size The size of the memory pool in bytes.
 */
void mem_init(size_t size) {
    if (size == 0) {
        printf("Size must be greater than zero.\n");
        exit(1); // Can't proceed with a pool size of zero
    }

    // Allocate the memory pool
    memory_pool = (char*)malloc(size);
    if (memory_pool == NULL) {
        printf("Memory pool allocation failed!\n");
        exit(1); // Critical failure; can't continue
    }

    // Allocate the allocation map (one bool per byte)
    allocation_map = (bool*)malloc(size * sizeof(bool));
    if (allocation_map == NULL) {
        printf("Allocation map creation failed!\n");
        free(memory_pool); // Clean up before exiting
        exit(1);
    }

    // Allocate the allocation size map
    allocation_size_map = (size_t*)malloc(size * sizeof(size_t));
    if (allocation_size_map == NULL) {
        printf("Allocation size map creation failed!\n");
        free(memory_pool);
        free(allocation_map);
        exit(1);
    }

    // Initialize allocation maps to indicate all memory is free
    for (size_t i = 0; i < size; i++) {
        allocation_map[i] = false;
        allocation_size_map[i] = 0;
    }

    pool_size = size;               // Set the total pool size
    total_allocated_memory = 0;    // No memory allocated yet

    printf("Memory pool of size %zu bytes initialized.\n", size);
}

/**
 * @brief Allocate a block of memory from the pool.
 *
 * Uses the first-fit strategy to find a contiguous block of the requested size.
 *
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void* mem_alloc(size_t size) {
    if (size == 0) {
        printf("Cannot allocate 0 bytes.\n");
        return NULL;
    }

    if (memory_pool == NULL || allocation_map == NULL || allocation_size_map == NULL) {
        printf("Memory pool is not initialized.\n");
        return NULL;
    }

    if (total_allocated_memory + size > pool_size) {
        printf("Not enough memory available.\n");
        return NULL;
    }

    size_t free_blocks = 0;
    size_t start_index = 0;

    // First-fit strategy with a double-check to prevent overlapping issues
    for (size_t i = 0; i < pool_size; i++) {
        if (!allocation_map[i]) {
            if (free_blocks == 0) {
                start_index = i;  // Mark potential start of free region
            }
            free_blocks++;

            if (free_blocks == size) {
                // Double-check the entire region is actually free
                bool is_region_free = true;
                for (size_t j = start_index; j < start_index + size; j++) {
                    if (allocation_map[j]) {
                        is_region_free = false;
                        break;
                    }
                }

                if (is_region_free) {
                    // Mark region as allocated
                    for (size_t j = start_index; j < start_index + size; j++) {
                        allocation_map[j] = true;
                    }
                    allocation_size_map[start_index] = size;  // Record the size
                    total_allocated_memory += size;

                    printf("Allocated %zu bytes at index %zu. Total allocated: %zu bytes.\n", size, start_index, total_allocated_memory);
                    return memory_pool + start_index;
                } else {
                    // Reset free_blocks count and continue searching
                    free_blocks = 0;
                }
            }
        } else {
            free_blocks = 0;  // Reset if the current block is not free
        }
    }

    // No suitable block was found
    printf("Not enough contiguous memory available to allocate %zu bytes.\n", size);
    return NULL;
}


/**
 * @brief Free a previously allocated block of memory.
 *
 * Marks the block as free and updates the allocation maps.
 *
 * @param block Pointer to the memory block to free.
 */
void mem_free(void* block) {
    if (block == NULL || (char*)block < memory_pool || (char*)block >= memory_pool + pool_size) {
        printf("Invalid block pointer. It does not belong to the memory pool.\n");
        return; // Can't free memory outside the pool
    }

    size_t start_index = (char*)block - memory_pool; // Calculate the index in the pool for bytes

    if (!allocation_map[start_index]) {
        printf("Block at index %zu is already free.\n", start_index);
        return; // Block is already free
    }

    size_t size = allocation_size_map[start_index];
    if (size == 0) {
        printf("No allocation size recorded for block at index %zu.\n", start_index);
        return; // Inconsistent state
    }

    // Mark the blocks as free
    for (size_t i = start_index; i < start_index + size; i++) {
        allocation_map[i] = false;
        allocation_size_map[i] = 0;
    }

    total_allocated_memory -= size;
    printf("Memory block freed. Freed %zu bytes. Total allocated: %zu bytes.\n", size, total_allocated_memory);
}

/**
 * @brief Resize an allocated memory block.
 *
 * Attempts to resize the block in place; if not possible, allocates a new block,
 * copies the data, and frees the old block.
 *
 * @param block Pointer to the memory block to resize.
 * @param new_size The new size in bytes.
 * @return Pointer to the resized memory block, or NULL if resizing fails.
 */
void* mem_resize(void* block, size_t new_size) {
    if (block == NULL) {
        // If block is NULL, behave like mem_alloc
        return mem_alloc(new_size);
    }

    if (new_size == 0) {
        // If new size is zero, free the block
        mem_free(block);
        return NULL;
    }

    size_t start_index = (char*)block - memory_pool; // Find the block's start index
    size_t current_size = allocation_size_map[start_index];

    if (current_size == 0) {
        printf("No allocation size recorded for block at index %zu.\n", start_index);
        return NULL; // Can't resize an untracked block
    }

    if (new_size <= current_size) {
        // Shrinking the block; free the extra space
        for (size_t i = start_index + new_size; i < start_index + current_size; i++) {
            allocation_map[i] = false;
            allocation_size_map[i] = 0;
        }
        total_allocated_memory -= (current_size - new_size);
        allocation_size_map[start_index] = new_size;

        printf("Resized block at index %zu to %zu bytes. Total allocated: %zu bytes.\n", start_index, new_size, total_allocated_memory);
        return block; // Return the same block since it's resized in place
    }

    // Check if we can expand the block in place
    size_t required_size = new_size - current_size;
    size_t i;
   
    for (i = start_index + current_size; i < start_index + new_size; i++) {
        if (i >= pool_size || allocation_map[i]) {
            break; // Can't expand further
        }
    }

    if (i == start_index + new_size) {
        // Enough space to expand in place
        for (size_t j = start_index + current_size; j < start_index + new_size; j++) {
            allocation_map[j] = true;
        }
        allocation_size_map[start_index] = new_size;
        total_allocated_memory += (new_size - current_size);

        printf("Expanded block at index %zu to %zu bytes. Total allocated: %zu bytes.\n", start_index, new_size, total_allocated_memory);
        return block; // Successfully resized in place
    }

    // If in-place expansion isn't possible, allocate a new block
    void* new_block = mem_alloc(new_size);
    if (new_block) {
        memcpy(new_block, block, current_size); // Copy existing data to the new block
        mem_free(block); // Free the old block

        printf("Resized block by allocating new block of %zu bytes and freeing old block. Total allocated: %zu bytes.\n", new_size, total_allocated_memory);
    }

    return new_block; // Return the new block or NULL if allocation failed
}

/**
 * @brief Deinitialize the memory pool, freeing all allocated resources.
 *
 * Frees the memory pool and allocation maps, resetting all tracking variables.
 */
void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);
        memory_pool = NULL;
    }

    if (allocation_map != NULL) {
        free(allocation_map);
        allocation_map = NULL;
    }

    if (allocation_size_map != NULL) {
        free(allocation_size_map);
        allocation_size_map = NULL;
    }

    total_allocated_memory = 0;
    pool_size = 0;

    printf("Memory pool deinitialized.\n");
}

/**
 * @brief Print the current allocation map for debugging purposes.
 *
 * Displays a simple binary map where '1' indicates allocated and '0' indicates free.
 */
void print_allocation_map() {
    printf("Allocation Map: ");
    for (size_t i = 0; i < pool_size; i++) {
        printf("%d", allocation_map[i]);
    }
    printf("\n");
}
