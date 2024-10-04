#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdint.h>
#include <stddef.h>

// Node structure for the singly linked list
typedef struct Node {
    uint16_t data;       // Stores the data as an unsigned 16-bit integer
    struct Node* next;   // Pointer to the next node in the list
} Node;

// Initialization function
/**
 * @brief Initializes the linked list and the memory manager.
 *
 * This function initializes the memory manager with a specified size and sets the head of the list to NULL.
 *
 * @param head Pointer to the head of the linked list.
 * @param size Size of the memory pool in bytes.
 */
void list_init(Node** head, size_t size);

// Insertion functions
/**
 * @brief Inserts a new node with the specified data at the end of the list.
 *
 * @param head Pointer to the head of the linked list.
 * @param data Data to be inserted into the new node.
 */
void list_insert(Node** head, uint16_t data);

/**
 * @brief Inserts a new node with the specified data immediately after the given node.
 *
 * @param prev_node Pointer to the node after which the new node will be inserted.
 * @param data Data to be inserted into the new node.
 */
void list_insert_after(Node* prev_node, uint16_t data);

/**
 * @brief Inserts a new node with the specified data immediately before the given node.
 *
 * @param head Pointer to the head of the linked list.
 * @param next_node Pointer to the node before which the new node will be inserted.
 * @param data Data to be inserted into the new node.
 */
void list_insert_before(Node** head, Node* next_node, uint16_t data);

// Deletion function
/**
 * @brief Deletes the first node with the specified data from the list.
 *
 * @param head Pointer to the head of the linked list.
 * @param data Data of the node to be deleted.
 */
void list_delete(Node** head, uint16_t data);

// Search function
/**
 * @brief Searches for the first node with the specified data.
 *
 * @param head Pointer to the head of the linked list.
 * @param data Data to search for.
 * @return Pointer to the found node, or NULL if not found.
 */
Node* list_search(Node** head, uint16_t data);

// Display functions
/**
 * @brief Displays all elements in the linked list.
 *
 * @param head Pointer to the head of the linked list.
 */
void list_display(Node** head);

/**
 * @brief Displays elements in the linked list between two specified nodes.
 *
 * @param head Pointer to the head of the linked list.
 * @param start_node Pointer to the starting node (inclusive). If NULL, starts from the head.
 * @param end_node Pointer to the ending node (inclusive). If NULL, ends at the last node.
 */
void list_display_range(Node** head, Node* start_node, Node* end_node);

// Nodes count function
/**
 * @brief Counts the number of nodes in the linked list.
 *
 * @param head Pointer to the head of the linked list.
 * @return The total number of nodes in the list.
 */
int list_count_nodes(Node** head);

// Cleanup function
/**
 * @brief Cleans up the linked list by freeing all nodes and deinitializing the memory manager.
 *
 * @param head Pointer to the head of the linked list.
 */
void list_cleanup(Node** head);

#endif // LINKED_LIST_H
