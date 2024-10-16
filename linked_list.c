#include "linked_list.h"
#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * @brief Redirects stdout to /dev/null to suppress unwanted output. Basically just deletes them.
 *
 * @return FILE* pointing to the original stdout before redirection, or NULL on failure.
 */
FILE* redirect_stdout_to_null() {
    fflush(stdout); // Make sure all pending output is written out

    // Open /dev/null so we can discard any output sent to stdout
    int dev_null = open("/dev/null", O_WRONLY);// Write only mode
    if (dev_null == -1) {
        perror("open"); // Print error if opening /dev/null fails
        return NULL;
    }

    // Save the current stdout file descriptor, fileno return the file descripter
    int saved_stdout_fd = dup(fileno(stdout));
    if (saved_stdout_fd == -1) {
        perror("dup"); // Print error if duplicating fails
        close(dev_null); // Clean up before returning
        return NULL;
    }

    // Redirect stdout to /dev/null
    if (dup2(dev_null, fileno(stdout)) == -1) {
        perror("dup2"); // Print error if redirection fails
        close(dev_null);
        close(saved_stdout_fd);
        return NULL;
    }

    close(dev_null); // No longer need the original /dev/null file descriptor

    // Turn the saved file descriptor back into a FILE* stream
    FILE* saved_stdout = fdopen(saved_stdout_fd, "w");
    if (saved_stdout == NULL) {
        perror("fdopen"); // Print error if converting fails
        close(saved_stdout_fd);
        return NULL;
    }

    return saved_stdout; // Return the original stdout stream
}

/**
 * @brief Restores stdout from a previously saved FILE* stream.
 *
 * @param saved_stdout_fp FILE* pointing to the original stdout.
 */
void restore_stdout_from_null(FILE* saved_stdout_fp) {
    if (saved_stdout_fp == NULL) {
        fprintf(stderr, "Error: saved_stdout_fp is NULL in restore_stdout_from_null.\n");
        return;
    }

    fflush(stdout); // Flush any output that's been redirected

    // Restore the original stdout file descriptor
    if (dup2(fileno(saved_stdout_fp), fileno(stdout)) == -1) {
        perror("dup2"); // Print error if restoration fails
    }

    fclose(saved_stdout_fp); // Close the saved stdout stream
}

/**
 * @brief Initializes the linked list and the memory manager.
 *
 * Sets up the memory manager with the given size and initializes the list head.
 *
 * @param head Pointer to the head of the linked list.
 * @param size Size of the memory pool in bytes.
 */
void list_init(Node** head, size_t size) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_init.\n");
        exit(EXIT_FAILURE); // Can't proceed without a valid head pointer
    }

    // Temporarily hide stdout to prevent mem_init from printing debug info
    FILE* saved_stdout = redirect_stdout_to_null();
    if (saved_stdout == NULL) {
        printf("Error: Failed to redirect stdout in list_init.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the memory manager with the specified size
    mem_init(size);

    // Bring stdout back to normal
    restore_stdout_from_null(saved_stdout);

    *head = NULL; // Start with an empty list
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

    // Hide stdout to prevent mem_alloc from printing debug info
    FILE* saved_stdout = redirect_stdout_to_null();
    if (saved_stdout == NULL) {
        printf("Error: Failed to redirect stdout in list_insert.\n");
        return;
    }

    // Allocate memory for the new node using the custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));// Size of bytes

    // Restore stdout after allocation
    restore_stdout_from_null(saved_stdout);

    if (new_node == NULL) {
        printf("Error: Memory allocation failed in list_insert.\n");
        return;
    }

    // Set up the new node's data and next pointer
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        // If the list is empty, the new node becomes the head
        *head = new_node;
    } else {
        // Otherwise, find the last node
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Link the new node at the end
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

    // Hide stdout to prevent mem_alloc from printing debug info
    FILE* saved_stdout = redirect_stdout_to_null();
    if (saved_stdout == NULL) {
        printf("Error: Failed to redirect stdout in list_insert_after.\n");
        return;
    }

    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));

    // Restore stdout after allocation
    restore_stdout_from_null(saved_stdout);

    if (new_node == NULL) {
        printf("Error: Memory allocation failed in list_insert_after.\n");
        return;
    }

    // Set up the new node's data and link it after prev_node
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

    // Hide stdout to prevent mem_alloc from printing debug info
    FILE* saved_stdout = redirect_stdout_to_null();
    if (saved_stdout == NULL) {
        printf("Error: Failed to redirect stdout in list_insert_before.\n");
        return;
    }

    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));

    // Restore stdout after allocation
    restore_stdout_from_null(saved_stdout);

    if (new_node == NULL) {
        printf("Error: Memory allocation failed in list_insert_before.\n");
        return;
    }

    // Set the new node's data
    new_node->data = data;

    if (*head == next_node) {
        // If we're inserting before the head, update the head pointer
        new_node->next = *head;
        *head = new_node;
    } else {
        // Find the node just before next_node
        Node* current = *head;
        while (current != NULL && current->next != next_node) {
            current = current->next;
        }

        if (current == NULL) {
            printf("Error: next_node not found in the list.\n");
            // Free the allocated node since we can't insert it
            FILE* saved_free_stdout = redirect_stdout_to_null();
            mem_free(new_node);
            restore_stdout_from_null(saved_free_stdout);
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

    // Search for the node to delete
    while (current != NULL && current->data != data) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Error: Node with data %u not found in list_delete.\n", data);
        return;
    }

    if (prev == NULL) {
        // If the node to delete is the head, update the head pointer
        *head = current->next;
    } else {
        // Otherwise, unlink the node from the list
        prev->next = current->next;
    }

    // Hide stdout to prevent mem_free from printing debug info
    FILE* saved_stdout = redirect_stdout_to_null();
    mem_free(current); // Free the memory of the deleted node
    restore_stdout_from_null(saved_stdout);
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

    // Look through the list for the data
    while (current != NULL) {
        if (current->data == data) {
            return current; // Found the node
        }
        current = current->next;
    }

    // Didn't find the node
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
    printf("]"); // Keeping output consistent without adding a newline
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

    // If a start_node is provided, find it first
    if (start_node != NULL) {
        while (current != NULL && current != start_node) {
            current = current->next;
        }
        if (current == NULL) {
            printf("]"); // start_node not found; nothing to display
            return;
        }
    }

    // Now, print nodes until we reach end_node
    while (current != NULL) {
        printf("%u", current->data);
        if (current == end_node) {
            break; // Reached the end of the range
        }
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]"); // Consistent output formatting
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

    // Simply traverse the list and count nodes
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
        Node* temp = current; // Making sure the memory isn't freed twice 
        current = current->next;

        // Hide stdout to prevent mem_free from printing debug info
        FILE* saved_stdout = redirect_stdout_to_null();
        mem_free(temp); // Free each node
        restore_stdout_from_null(saved_stdout);
    }

    *head = NULL; // Reset the head pointer

    // Finally, deinitialize the memory manager
    FILE* saved_deinit_stdout = redirect_stdout_to_null();
    mem_deinit();
    restore_stdout_from_null(saved_deinit_stdout);
}
