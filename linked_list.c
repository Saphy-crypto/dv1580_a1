#include "linked_list.h"
#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initializes the linked list and the memory manager.
 *
 * This function initializes the memory manager with a specified size and sets the head of the list to NULL.
 *
 * @param head Pointer to the head of the linked list.
 * @param size Size of the memory pool in bytes.
 */
void list_init(Node** head, size_t size) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_init.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize memory manager
    mem_init(size);

    // Initialize the head to NULL
    *head = NULL;
}

/**
 * @brief Inserts a new node with the specified data at the end of the list.
 *
 * @param head Pointer to the head of the linked list.
 * @param data Data to be inserted into the new node.
 */
void list_insert(Node** head, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_insert.\n");
        return;
    }

    // Allocate memory for the new node using custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed in list_insert.\n");
        return;
    }

    // Initialize the new node
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        // If the list is empty, set the new node as head
        *head = new_node;
    } else {
        // Traverse to the end of the list
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Insert the new node at the end
        current->next = new_node;
    }
}

/**
 * @brief Inserts a new node with the specified data immediately after the given node.
 *
 * @param prev_node Pointer to the node after which the new node will be inserted.
 * @param data Data to be inserted into the new node.
 */
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Error: prev_node is NULL in list_insert_after.\n");
        return;
    }

    // Allocate memory for the new node using custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed in list_insert_after.\n");
        return;
    }

    // Initialize the new node
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

/**
 * @brief Inserts a new node with the specified data immediately before the given node.
 *
 * @param head Pointer to the head of the linked list.
 * @param next_node Pointer to the node before which the new node will be inserted.
 * @param data Data to be inserted into the new node.
 */
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_insert_before.\n");
        return;
    }

    if (next_node == NULL) {
        printf("Error: next_node is NULL in list_insert_before.\n");
        return;
    }

    // Allocate memory for the new node using custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed in list_insert_before.\n");
        return;
    }

    // Initialize the new node
    new_node->data = data;

    if (*head == next_node) {
        // Inserting before the head node
        new_node->next = *head;
        *head = new_node;
    } else {
        // Traverse the list to find the node before next_node
        Node* current = *head;
        while (current != NULL && current->next != next_node) {
            current = current->next;
        }

        if (current == NULL) {
            printf("Error: next_node not found in the list.\n");
            mem_free(new_node);
            return;
        }

        // Insert the new node between current and next_node
        current->next = new_node;
        new_node->next = next_node;
    }
}

/**
 * @brief Deletes the first node with the specified data from the list.
 *
 * @param head Pointer to the head of the linked list.
 * @param data Data of the node to be deleted.
 */
void list_delete(Node** head, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_delete.\n");
        return;
    }

    if (*head == NULL) {
        printf("Error: Cannot delete from an empty list.\n");
        return;
    }

    Node* current = *head;
    Node* prev = NULL;

    // Traverse the list to find the node to delete
    while (current != NULL && current->data != data) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Error: Node with data %u not found in list_delete.\n", data);
        return;
    }

    if (prev == NULL) {
        // The node to delete is the head
        *head = current->next;
    } else {
        // The node to delete is in the middle or end
        prev->next = current->next;
    }

    // Free the memory of the deleted node using custom memory manager
    mem_free(current);
}

/**
 * @brief Searches for the first node with the specified data.
 *
 * @param head Pointer to the head of the linked list.
 * @param data Data to search for.
 * @return Pointer to the found node, or NULL if not found.
 */
Node* list_search(Node** head, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_search.\n");
        return NULL;
    }

    Node* current = *head;

    // Traverse the list to find the node
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }

    // Node not found
    return NULL;
}

/**
 * @brief Displays all elements in the linked list.
 *
 * @param head Pointer to the head of the linked list.
 */
void list_display(Node** head) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_display.\n");
        return;
    }

    printf("[");
    Node* current = *head;
    while (current != NULL) {
        printf("%u", current->data);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

/**
 * @brief Displays elements in the linked list between two specified nodes.
 *
 * @param head Pointer to the head of the linked list.
 * @param start_node Pointer to the starting node (inclusive). If NULL, starts from the head.
 * @param end_node Pointer to the ending node (inclusive). If NULL, ends at the last node.
 */
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_display_range.\n");
        return;
    }

    printf("[");
    Node* current = *head;

    // If start_node is specified, find it
    if (start_node != NULL) {
        while (current != NULL && current != start_node) {
            current = current->next;
        }
        if (current == NULL) {
            printf("]");
            return; // start_node not found
        }
    }

    // Iterate and print until end_node is reached
    while (current != NULL) {
        printf("%u", current->data);
        if (current == end_node) {
            break;
        }
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

/**
 * @brief Counts the number of nodes in the linked list.
 *
 * @param head Pointer to the head of the linked list.
 * @return The total number of nodes in the list.
 */
int list_count_nodes(Node** head) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_count_nodes.\n");
        return 0;
    }

    int count = 0;
    Node* current = *head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

/**
 * @brief Cleans up the linked list by freeing all nodes and deinitializing the memory manager.
 *
 * @param head Pointer to the head of the linked list.
 */
void list_cleanup(Node** head) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_cleanup.\n");
        return;
    }

    Node* current = *head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        mem_free(temp);
    }

    *head = NULL;

    // Deinitialize memory manager
    mem_deinit();
}
