#include "linked_list.h"
#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define POOL_SIZE 2024  // Define the size of the memory pool

//Pointer to the beginning of the memory pool
static char *memory_pool = NULL;
//Array to track which parts of the pool are allocated
static bool *allocation_map = NULL;

// Memory manager functions
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
        allocation_map[i] = false;  // Mark all blocks as free
    }

    printf("Memory pool of size %zu bytes initialized.\n", size);
}

void* mem_alloc(size_t size) {
    size_t free_blocks = 0;
    size_t start_index = 0;

    // Check if memory pool is initialized
    if (memory_pool == NULL || allocation_map == NULL) {
        printf("Memory pool is not initialized.\n");
        return NULL;
    }

    // First-fit strategy: find the first contiguous free block big enough
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
                return memory_pool + start_index;
            }
        } else {
            // Reset the free block counter if we hit an allocated block
            free_blocks = 0;
        }
    }

    // If we exit the loop, no suitable block was found
    printf("Not enough contiguous memory available to allocate %zu bytes.\n", size);
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

// Node structure for the linked list
//typedef struct Node {
//  uint16_t data;          // Stores the data as an unsigned 16-bit integer
//  struct Node* next;      // A pointer to the next node in the list
//} Node;

// Linked list functions
void list_init(Node** head, size_t size) {
    *head = NULL; //Set head to NULL indicating an empty list
    mem_init(size); //Initialize memory pool using custom memory manager
}

// Inserts a node at the end of the list
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node)); // Use custom memory manager
    if (!new_node) {
        printf("Memory allocation failed for new node!\n");
        return;
    }

    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node; // Insert as head if list is empty
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node; // Insert at the end of the list
    }
}

//Inserts a node immediately after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Previous node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed for new node!\n");
        return;
    }

    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

//Inserts a node immediately before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (*head == NULL || next_node == NULL) {
        printf("Invalid node pointers.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed for new node!\n");
        return;
    }
    new_node->data = data;

    if (*head == next_node) {
        // Insert new node as head
        new_node->next = *head;
        *head = new_node;
        return;
    }

    // Traverse to find the node before next_node
    Node* temp = *head;
    while (temp->next != NULL && temp->next != next_node) {
        temp = temp->next;
    }

    if (temp->next == next_node) {
        new_node->next = next_node;
        temp->next = new_node;
    } else {
        printf("Next node not found in the list.\n");
        mem_free(new_node); // Free allocated memory
    }
}

void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        printf("List is empty. Cannot delete.\n");
        return;
    }

    Node* temp = *head;
    Node* prev = NULL;

    // If the node to be deleted is the head node
    if (temp != NULL && temp->data == data) {
        *head = temp->next; // Move head to the next node
        mem_free(temp); // Free the old head
        return;
    }

    // Search for the node to be deleted
    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    // If data is not found
    if (temp == NULL) {
        printf("Node with data %d not found.\n", data);
        return;
    }

    // Remove the node from the list
    prev->next = temp->next;
    mem_free(temp); // Free the node
}

Node* list_search(Node** head, uint16_t data) {
    Node* temp = *head;

    while (temp != NULL) {
        if (temp->data == data) {
            return temp; // Return node if data matches
        }
        temp = temp->next;
    }
    return NULL; // Return NULL if data not found
}

void list_display(Node** head) {
    Node* temp = *head;
    printf("[");

    while (temp != NULL) {
        printf("%d", temp->data);
        temp = temp->next;
        if (temp != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* temp = *head;

    // Start from the beginning if start_node is NULL
    if (start_node == NULL) {
        start_node = *head;
    }

    // Traverse to start_node
    while (temp != NULL && temp != start_node) {
        temp = temp->next;
    }

    // Display nodes from start_node to end_node
    printf("[");
    while (temp != NULL && temp != end_node->next) {
        printf("%d", temp->data);
        if (temp->next != NULL && temp != end_node) {
            printf(", ");
        }
        temp = temp->next;
    }
    printf("]\n");
}

int list_count_nodes(Node** head) {
    int count = 0;
    Node* temp = *head;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

void list_cleanup(Node** head) {
    Node* temp = *head;
    Node* next_node;

    while (temp != NULL) {
        next_node = temp->next;
        mem_free(temp);
        temp = next_node;
    }
    *head = NULL; // Set head to NULL after cleanup
}

