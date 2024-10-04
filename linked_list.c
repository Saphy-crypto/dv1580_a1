// File: src/linked_list.c

#include "linked_list.h"
#include "memory_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initializes the linked list.
 */
void list_init(Node** head, size_t size) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_init.\n");
        return;
    }

    *head = NULL;

    if (!mem_init(size)) {
        printf("Error: Memory manager initialization failed in list_init.\n");
        return;
    }

    printf("Linked list initialized with memory pool size %zu bytes.\n", size);
}

/**
 * Inserts a new node at the end of the list.
 */
void list_insert(Node** head, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_insert.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed for new node in list_insert!\n");
        return;
    }

    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        printf("Inserted first node with data %u.\n", data);
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
        printf("Inserted node with data %u at the end.\n", data);
    }
}

/**
 * Inserts a new node after the specified node.
 */
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Error: prev_node is NULL in list_insert_after.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed for new node in list_insert_after!\n");
        return;
    }

    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;

    printf("Inserted node with data %u after node with data %u.\n", data, prev_node->data);
}

/**
 * Inserts a new node before the specified node.
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

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed for new node in list_insert_before!\n");
        return;
    }

    new_node->data = data;

    // If the next_node is the head
    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
        printf("Inserted node with data %u before the head node.\n", data);
        return;
    }

    // Find the node before next_node
    Node* temp = *head;
    while (temp != NULL && temp->next != next_node) {
        temp = temp->next;
    }

    if (temp != NULL && temp->next == next_node) {
        new_node->next = next_node;
        temp->next = new_node;
        printf("Inserted node with data %u before node with data %u.\n", data, next_node->data);
    } else {
        printf("Error: next_node with data %u not found in list_insert_before.\n", next_node->data);
        mem_free(new_node);
    }
}

/**
 * Deletes the first node with the specified data.
 */
void list_delete(Node** head, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_delete.\n");
        return;
    }

    if (*head == NULL) {
        printf("List is empty. Cannot delete data %u.\n", data);
        return;
    }

    Node* temp = *head;
    Node* prev = NULL;

    // If head node holds the data
    if (temp != NULL && temp->data == data) {
        *head = temp->next;
        mem_free(temp);
        printf("Deleted node with data %u from head.\n", data);
        return;
    }

    // Search for the data to delete
    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    // Data not found
    if (temp == NULL) {
        printf("Node with data %u not found. Cannot delete.\n", data);
        return;
    }

    // Unlink the node
    prev->next = temp->next;
    mem_free(temp);
    printf("Deleted node with data %u from the list.\n", data);
}

/**
 * Searches for the first node with the specified data.
 */
Node* list_search(Node** head, uint16_t data) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_search.\n");
        return NULL;
    }

    Node* temp = *head;
    while (temp != NULL) {
        if (temp->data == data) {
            printf("Found node with data %u.\n", data);
            return temp;
        }
        temp = temp->next;
    }

    printf("Node with data %u not found.\n", data);
    return NULL;
}

/**
 * Displays the entire linked list.
 */
void list_display(Node** head) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_display.\n");
        return;
    }

    Node* temp = *head;
    printf("Full list: [");

    while (temp != NULL) {
        printf("%u", temp->data);
        temp = temp->next;
        if (temp != NULL) {
            printf(", ");
        }
    }

    printf("]\n");
}

/**
 * Displays a range of nodes from start_node to end_node inclusive.
 */
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_display_range.\n");
        return;
    }

    if (start_node == NULL) {
        printf("Error: start_node is NULL in list_display_range.\n");
        return;
    }

    if (end_node == NULL) {
        printf("Error: end_node is NULL in list_display_range.\n");
        return;
    }

    Node* temp = *head;

    // Find the start_node
    while (temp != NULL && temp != start_node) {
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("Start node not found in list_display_range.\n");
        return;
    }

    printf("List Range: [");

    while (temp != NULL && temp != end_node->next) {
        printf("%u", temp->data);
        temp = temp->next;
        if (temp != NULL && temp != end_node->next) {
            printf(", ");
        }
    }

    printf("]\n");
}

/**
 * Counts the number of nodes in the list.
 */
int list_count_nodes(Node** head) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_count_nodes.\n");
        return 0;
    }

    int count = 0;
    Node* temp = *head;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    printf("Total nodes in list: %d.\n", count);
    return count;
}

/**
 * Cleans up the list by freeing all nodes and deinitializing the memory manager.
 */
void list_cleanup(Node** head) {
    if (head == NULL) {
        printf("Error: head pointer is NULL in list_cleanup.\n");
        return;
    }

    Node* temp = *head;
    Node* next_node;

    while (temp != NULL) {
        next_node = temp->next;
        mem_free(temp);
        temp = next_node;
    }

    *head = NULL;

    mem_deinit();

    printf("Linked list cleaned up and memory pool deinitialized.\n");
}
