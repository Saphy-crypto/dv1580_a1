#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

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
        exit(1);
    }

    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t alloc_size = ((size + page_size - 1) / page_size) * page_size;

    // Allocate the memory pool using mmap
    memory_pool = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        perror("Memory pool allocation failed");
        exit(1);
    }

    // Allocate the allocation map
    allocation_map = mmap(NULL, alloc_size * sizeof(bool), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocation_map == MAP_FAILED) {
        perror("Allocation map creation failed");
        munmap(memory_pool, alloc_size);
        exit(1);
    }

    // Allocate the allocation size map
    allocation_size_map = mmap(NULL, alloc_size * sizeof(size_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocation_size_map == MAP_FAILED) {
        perror("Allocation size map creation failed");
        munmap(memory_pool, alloc_size);
        munmap(allocation_map, alloc_size * sizeof(bool));
        exit(1);
    }

    // Initialize allocation maps
    memset(allocation_map, 0, alloc_size * sizeof(bool));
    memset(allocation_size_map, 0, alloc_size * sizeof(size_t));

    pool_size = size;
    total_allocated_memory = 0;

    printf("Memory pool of size %zu bytes initialized at %p.\n", size, memory_pool);
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
        // Handle zero-size allocation by returning a valid pointer without reserving memory.
        // We can simply return the start of the memory pool for simplicity.
        printf("Allocating 0 bytes. Returning a placeholder pointer.\n");
        return memory_pool;  // Return a valid pointer, such as the start of the memory pool.
    }

    // Ensure the memory pool is initialized
    if (memory_pool == NULL || allocation_map == NULL || allocation_size_map == NULL) {
        printf("Memory pool is not initialized.\n");
        return NULL;
    }

    // Check if there's enough memory left
    if (total_allocated_memory + size > pool_size) {
        printf("Not enough memory available to allocate %zu bytes. Total allocated: %zu bytes.\n", size, total_allocated_memory);
        return NULL;
    }

    size_t free_blocks = 0;  // Counts consecutive free blocks
    size_t start_index = 0;  // Starting index of a potential free block

    // First-fit strategy: find the first block that fits
    for (size_t i = 0; i < pool_size; i++) {
        if (!allocation_map[i]) { // If the block is free
            if (free_blocks == 0) {
                start_index = i; // Potential start of free block
            }
            free_blocks++;

            if (free_blocks == size) {
                // Found a suitable block; mark it as allocated
                for (size_t j = start_index; j < start_index + size; j++) {
                    allocation_map[j] = true;
                }
                allocation_size_map[start_index] = size; // Record the size
                total_allocated_memory += size;

                printf("Allocated %zu bytes at index %zu. Total allocated: %zu bytes.\n", size, start_index, total_allocated_memory);
                return memory_pool + start_index; // Return pointer to allocated memory
            }
        } else {
            free_blocks = 0; // Reset if block is not free
        }
    }

    // If we reach here, no suitable block was found
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
    }

    allocation_size_map[start_index] = 0;
    total_allocated_memory -= size;

    // Try to merge with the previous block if it is free
    if (start_index > 0 && !allocation_map[start_index - 1]) {
        size_t prev_index = start_index - 1;
        while (prev_index > 0 && allocation_size_map[prev_index] == 0) {
            prev_index--;
        }
        if (allocation_size_map[prev_index] > 0) {
            size_t prev_size = allocation_size_map[prev_index];
            allocation_size_map[prev_index] += size;
            total_allocated_memory -= size;
            allocation_size_map[start_index] = 0;
        }
    }

    // Try to merge with the next block if it is free
    size_t next_index = start_index + size;
    if (next_index < pool_size && !allocation_map[next_index]) {
        size_t next_size = allocation_size_map[next_index];
        if (next_size > 0) {
            allocation_size_map[start_index] += next_size;
            allocation_size_map[next_index] = 0;
        }
    }

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
        munmap(memory_pool, pool_size);
        memory_pool = NULL;
    }

    if (allocation_map != NULL) {
        munmap(allocation_map, pool_size * sizeof(bool));
        allocation_map = NULL;
    }

    if (allocation_size_map != NULL) {
        munmap(allocation_size_map, pool_size * sizeof(size_t));
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
