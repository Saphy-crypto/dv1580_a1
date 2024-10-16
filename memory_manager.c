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
//test
/**
 * @brief Initialize the memory pool with a specified size.
 *
 * This function sets up the memory pool by allocating a contiguous block of memory
 * using `mmap`. It also initializes two auxiliary maps:
 *  - `allocation_map` to track which bytes are allocated.
 *  - `allocation_size_map` to record the size of each allocation.
 *
 * The allocation size is rounded up to the system's page size to ensure proper alignment.
 *
 * @param size The desired size of the memory pool in bytes.
 */
void mem_init(size_t size) {
    // Ensure the requested size is greater than zero
    if (size == 0) {
        printf("Error: Memory pool size must be greater than zero.\n");
        exit(1); // Exit if size is invalid
    }

    // Retrieve the system's page size to align the memory allocation
    size_t page_size = sysconf(_SC_PAGESIZE);
    // Calculate the allocation size rounded up to the nearest multiple of page_size
    size_t alloc_size = ((size + page_size - 1) / page_size) * page_size;

    // Allocate the memory pool using mmap for a contiguous block of memory
    memory_pool = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        perror("Error: Failed to allocate memory pool with mmap");
        exit(1); // Critical failure; cannot proceed
    }

    // Allocate the allocation map using mmap
    // This map keeps track of whether each byte in the pool is allocated (true) or free (false)
    allocation_map = mmap(NULL, alloc_size * sizeof(bool), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocation_map == MAP_FAILED) {
        perror("Error: Failed to create allocation map with mmap");
        munmap(memory_pool, alloc_size); // Clean up the previously allocated memory pool
        exit(1); // Critical failure; cannot proceed
    }

    // Allocate the allocation size map using mmap
    // This map records the size of each allocated block starting at a specific index
    allocation_size_map = mmap(NULL, alloc_size * sizeof(size_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocation_size_map == MAP_FAILED) {
        perror("Error: Failed to create allocation size map with mmap");
        munmap(memory_pool, alloc_size);        // Clean up memory pool
        munmap(allocation_map, alloc_size * sizeof(bool)); // Clean up allocation map
        exit(1); // Critical failure; cannot proceed
    }

    // Initialize the allocation maps to indicate that all memory is free
    memset(allocation_map, 0, alloc_size * sizeof(bool));      // Set all bytes as free (false)
    memset(allocation_size_map, 0, alloc_size * sizeof(size_t)); // Initialize sizes to zero

    // Set the pool size and reset the total allocated memory counter
    pool_size = size;
    total_allocated_memory = 0;

    // Inform the user that the memory pool has been successfully initialized
    printf("Memory pool of size %zu bytes initialized at address %p.\n", size, (void*)memory_pool);
}

/**
 * @brief Allocate a block of memory from the pool.
 *
 * This function uses the first-fit strategy to find a contiguous block of the requested size
 * within the memory pool. If a suitable block is found, it marks the corresponding bytes as
 * allocated in the `allocation_map` and records the block size in `allocation_size_map`.
 *
 * Special handling is provided for zero-size allocations, which return a valid pointer without
 * reserving any memory.
 *
 * @param size The size of memory to allocate in bytes.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void* mem_alloc(size_t size) {
    // Handle zero-size allocations by returning a valid pointer without reserving memory
    if (size == 0) {
        printf("Allocating 0 bytes. Returning a placeholder pointer.\n");
        return memory_pool; // Return the start of the memory pool as a placeholder
    }

    // Ensure the memory pool and maps are initialized
    if (memory_pool == NULL || allocation_map == NULL || allocation_size_map == NULL) {
        printf("Error: Memory pool is not initialized.\n");
        return NULL; // Allocation cannot proceed without initialization
    }

    // Check if the requested size exceeds the total pool size
    if (size > pool_size) {
        printf("Error: Requested allocation size (%zu bytes) exceeds pool size (%zu bytes).\n", size, pool_size);
        return NULL; // Cannot allocate more than the pool size
    }

    size_t free_blocks = 0;  // Counter for consecutive free blocks
    size_t start_index = 0;  // Starting index of a potential free block

    // Iterate through the allocation map to find the first suitable free block
    for (size_t i = 0; i < pool_size; i++) {
        if (!allocation_map[i]) { // Check if the current byte is free
            if (free_blocks == 0) {
                start_index = i; // Potential start of a free block
            }
            free_blocks++;

            // If enough consecutive free blocks are found
            if (free_blocks == size) {
                // Mark the block as allocated in the allocation map
                for (size_t j = start_index; j < start_index + size; j++) {
                    allocation_map[j] = true;
                }
                // Record the size of the allocation in the size map
                allocation_size_map[start_index] = size;
                // Update the total allocated memory
                total_allocated_memory += size;

                printf("Allocated %zu bytes at index %zu. Total allocated: %zu bytes.\n", size, start_index, total_allocated_memory);
                return memory_pool + start_index; // Return a pointer to the allocated memory
            }
        } else {
            free_blocks = 0; // Reset the counter if a used byte is encountered
        }
    }

    // If no suitable block is found, inform the user
    printf("Error: Not enough contiguous memory available to allocate %zu bytes.\n", size);
    return NULL; // Allocation failed due to insufficient memory
}

/**
 * @brief Free a previously allocated block of memory.
 *
 * This function marks the specified block as free in the `allocation_map` and clears
 * its size entry in `allocation_size_map`. It also attempts to merge the freed block
 * with adjacent free blocks to reduce fragmentation.
 *
 * @param block Pointer to the memory block to free.
 */
void mem_free(void* block) {
    // Validate the block pointer to ensure it belongs to the memory pool
    if (block == NULL || (char*)block < memory_pool || (char*)block >= memory_pool + pool_size) {
        printf("Error: Invalid block pointer. It does not belong to the memory pool.\n");
        return; // Cannot free memory outside the pool
    }

    // Calculate the starting index of the block within the memory pool
    size_t start_index = (char*)block - memory_pool;

    // Check if the block is already free
    if (!allocation_map[start_index]) {
        printf("Warning: Block at index %zu is already free.\n", start_index);
        return; // Block is already free; no action needed
    }

    // Retrieve the size of the allocated block from the size map
    size_t size = allocation_size_map[start_index];
    if (size == 0) {
        printf("Error: No allocation size recorded for block at index %zu.\n", start_index);
        return; // Inconsistent state; cannot proceed
    }

    // Mark each byte of the block as free in the allocation map
    for (size_t i = start_index; i < start_index + size; i++) {
        allocation_map[i] = false;
    }

    // Clear the size entry for the block in the size map
    allocation_size_map[start_index] = 0;
    // Update the total allocated memory
    total_allocated_memory -= size;

    // Attempt to merge with the previous block if it is free
    if (start_index > 0 && !allocation_map[start_index - 1]) {
        size_t prev_index = start_index - 1;

        // Find the start of the previous free block
        while (prev_index > 0 && allocation_size_map[prev_index] == 0) {
            prev_index--;
        }

        // If a previous allocated block is found, merge with it
        if (allocation_size_map[prev_index] > 0) {
            size_t prev_size = allocation_size_map[prev_index];
            // Merge the current free block with the previous block
            allocation_size_map[prev_index] += size;
            // Clear the current block's size entry
            allocation_size_map[start_index] = 0;
            // Update the total allocated memory
            total_allocated_memory -= size;
        }
    }

    // Attempt to merge with the next block if it is free
    size_t next_index = start_index + size;
    if (next_index < pool_size && !allocation_map[next_index]) {
        size_t next_size = allocation_size_map[next_index];
        if (next_size > 0) {
            // Merge the current free block with the next block
            allocation_size_map[start_index] += next_size;
            // Clear the next block's size entry
            allocation_size_map[next_index] = 0;
        }
    }

    // Inform the user that the block has been successfully freed
    printf("Memory block freed. Freed %zu bytes. Total allocated: %zu bytes.\n", size, total_allocated_memory);
}

/**
 * @brief Resize an allocated memory block.
 *
 * This function attempts to resize an existing memory block to a new size. If the block
 * can be resized in place (i.e., there is enough contiguous free space immediately
 * after it), it expands or shrinks the block accordingly. If in-place resizing is not
 * possible, it allocates a new block of the desired size, copies the data, and frees
 * the original block.
 *
 * @param block Pointer to the memory block to resize.
 * @param new_size The new desired size of the memory block in bytes.
 * @return Pointer to the resized memory block, or NULL if resizing fails.
 */
void* mem_resize(void* block, size_t new_size) {
    // If the block pointer is NULL, behave like mem_alloc to allocate new memory
    if (block == NULL) {
        return mem_alloc(new_size);
    }

    // If the new size is zero, free the block and return NULL
    if (new_size == 0) {
        mem_free(block);
        return NULL;
    }

    // Calculate the starting index of the block within the memory pool
    size_t start_index = (char*)block - memory_pool;
    // Retrieve the current size of the allocated block
    size_t current_size = allocation_size_map[start_index];

    // Validate the current size to ensure the block is tracked
    if (current_size == 0) {
        printf("Error: No allocation size recorded for block at index %zu.\n", start_index);
        return NULL; // Cannot resize an untracked block
    }

    // If the new size is smaller or equal to the current size, shrink the block
    if (new_size <= current_size) {
        // Mark the excess bytes as free
        for (size_t i = start_index + new_size; i < start_index + current_size; i++) {
            allocation_map[i] = false;
        }
        // Update the total allocated memory
        total_allocated_memory -= (current_size - new_size);
        // Update the size map with the new size
        allocation_size_map[start_index] = new_size;

        printf("Resized block at index %zu to %zu bytes. Total allocated: %zu bytes.\n", start_index, new_size, total_allocated_memory);
        return block; // Return the same block as it was resized in place
    }

    // Calculate the additional size needed to expand the block
    size_t required_size = new_size - current_size;
    size_t i;

    // Check if the space immediately after the current block is free and sufficient
    for (i = start_index + current_size; i < start_index + new_size; i++) {
        if (i >= pool_size || allocation_map[i]) {
            break; // Cannot expand further due to boundary or allocated byte
        }
    }

    // If enough contiguous free space is available, expand the block in place
    if (i == start_index + new_size) {
        // Mark the additional bytes as allocated
        for (size_t j = start_index + current_size; j < start_index + new_size; j++) {
            allocation_map[j] = true;
        }
        // Update the size map with the new size
        allocation_size_map[start_index] = new_size;
        // Update the total allocated memory
        total_allocated_memory += (new_size - current_size);

        printf("Expanded block at index %zu to %zu bytes. Total allocated: %zu bytes.\n", start_index, new_size, total_allocated_memory);
        return block; // Successfully resized in place
    }

    // If in-place expansion is not possible, allocate a new block of the desired size
    void* new_block = mem_alloc(new_size);
    if (new_block) {
        // Copy the existing data to the new block
        memcpy(new_block, block, current_size);
        // Free the original block
        mem_free(block);

        printf("Resized block by allocating new block of %zu bytes and freeing old block. Total allocated: %zu bytes.\n", new_size, total_allocated_memory);
    }

    // Return the new block pointer, or NULL if allocation failed
    return new_block;
}

/**
 * @brief Deinitialize the memory pool and free all associated resources.
 *
 * This function releases the memory allocated for the memory pool, allocation map,
 * and allocation size map using `munmap`. It also resets all tracking variables
 * to their initial state.
 */
void mem_deinit() {
    // Free the memory pool if it was allocated
    if (memory_pool != NULL) {
        munmap(memory_pool, pool_size);
        memory_pool = NULL; // Reset the pointer to indicate deallocation
    }

    // Free the allocation map if it was allocated
    if (allocation_map != NULL) {
        munmap(allocation_map, pool_size * sizeof(bool));
        allocation_map = NULL; // Reset the pointer to indicate deallocation
    }

    // Free the allocation size map if it was allocated
    if (allocation_size_map != NULL) {
        munmap(allocation_size_map, pool_size * sizeof(size_t));
        allocation_size_map = NULL; // Reset the pointer to indicate deallocation
    }

    // Reset tracking variables
    total_allocated_memory = 0;
    pool_size = 0;

    // Inform the user that the memory pool has been deinitialized
    printf("Memory pool deinitialized.\n");
}

/**
 * @brief Print the current allocation map for debugging purposes.
 *
 * This function displays a simple binary map where '1' represents an allocated byte
 * and '0' represents a free byte within the memory pool. It helps in visualizing
 * the current state of memory allocations.
 */
void print_allocation_map() {
    printf("Allocation Map: ");
    for (size_t i = 0; i < pool_size; i++) {
        printf("%d", allocation_map[i]);
    }
    printf("\n");
}
