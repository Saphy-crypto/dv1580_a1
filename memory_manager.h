#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define POOL_SIZE 1024  // Define the size of the memory pool

// Pointer to the beginning of the memory pool
char *memory_pool = NULL;
// Array to track which parts of the pool are allocated
bool *allocation_map = NULL;

void mem_init(size_t size){
    // pointing to the start of the memory block,
    // we have been given by the function malloc.
    // The malloc function will return a pointer, pointing 
    // to the beginning of the block
    if (size == 0) {
    printf("Size must be greater than zero.\n");
    exit(1);
    }
    memory_pool = (char*)malloc(size);
    //if there is no memory available
    if(memory_pool == NULL){
        printf("Memory pool allocation failed!\n");
        exit(1);  // Exit the program if allocation fails
    }
    //This means we are allocating enough 
    // space for an array of 10 bool elements.
    //this is to check and see which parts of memory_pool are empty and can store 
    // bytes in
    allocation_map = (bool*)malloc(size * sizeof(bool));
    // check if there is enough memory for the allocation_map memory block,
    // if not exit the program and empty memory_pool
    if (allocation_map == NULL) {
        printf("Allocation map creation failed!\n");
        free(memory_pool);  // Free the memory pool
        exit(1);
    }
    // Initialize the allocation map to false (all memory is free)
    for (size_t i = 0; i < size; i++) {
        allocation_map[i] = false;  // Mark all blocks as free
    }
    printf("Memory pool of size %zu bytes initialized.\n", size);

}

void* mem_alloc(size_t size){
    size_t free_blocks = 0;  // To count contiguous free blocks
    size_t start_index = 0;  // To record where the free block starts

    // Check if memory pool is initialized
    if (memory_pool == NULL || allocation_map == NULL) {
        printf("Memory pool is not initialized.\n");
        return NULL;
    }

    // First-fit strategy: find first free block big enough
    for (size_t i = 0; i < POOL_SIZE; i++) {
        if (!allocation_map[i]) {
            // Start counting free blocks
            if (free_blocks == 0) {
                start_index = i;
            }
            free_blocks++;
            
            // If we've found enough free blocks, allocate memory
            if (free_blocks == size) {
                // Mark these blocks as allocated
                for (size_t j = start_index; j < start_index + size; j++) {
                    allocation_map[j] = true;
                }
                // Return pointer to the start of allocated block
                return memory_pool + start_index;
            }
        } else {
            // Reset free block counter if we hit an allocated block
            free_blocks = 0;
        }
    }

    // If we exit the loop, no suitable block was found
    printf("Not enough memory available to allocate %zu bytes.\n", size);
    return NULL;
}

void mem_free(void* block) {
    if (block == NULL || (char*)block < memory_pool || (char*)block >= memory_pool + POOL_SIZE) {
        printf("Invalid block pointer. It does not belong to the memory pool.\n");
        return;
    }
    size_t start_index = (char*)block - memory_pool;

    // Free contiguous allocated blocks starting from the block
    while (start_index < POOL_SIZE && allocation_map[start_index]) {
        allocation_map[start_index] = false;
        start_index++;
    }

    printf("Memory block freed.\n");
}

void* mem_resize(void* block, size_t new_size) {
    if (block == NULL) {
        // If block is NULL, just allocate a new block of the requested size
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
        return block;  // No need to move the block, it fits in place
    }

    // Check if the block can be expanded in place
    size_t i;
    for (i = start_index + current_size; i < start_index + new_size; i++) {
        if (i >= POOL_SIZE || allocation_map[i]) {
            break;  // Can't expand in place, need to move
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
        memcpy(new_block, block, current_size);  // Copy the old data
        mem_free(block);  // Free the old block
    }

    return new_block;  // Return the new block's pointer
}

void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);  // Free the memory pool
        memory_pool = NULL;
    }

    if (allocation_map != NULL) {
        free(allocation_map);  // Free the allocation map
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

