#include "linked_list.h"
#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>

// Initialize the linked list
void list_init(Node** head, size_t size) {
    *head = NULL;
    mem_init(size);
}

// Insert a new node at the end of the list
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Insert a new node after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Previous node cannot be NULL\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

// Insert a new node before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (head == NULL || *head == NULL || next_node == NULL) {
        printf("Invalid input\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != next_node && current->next != NULL) {
            current = current->next;
        }
        if (current->next == NULL) {
            printf("Next node not found in the list\n");
            mem_free(new_node);
            return;
        }
        new_node->next = next_node;
        current->next = new_node;
    }
}

// Delete a node with the given data
void list_delete(Node** head, uint16_t data) {
    if (head == NULL || *head == NULL) {
        return;
    }

    Node* temp = *head;
    Node* prev = NULL;

    if (temp != NULL && temp->data == data) {
        *head = temp->next;
        mem_free(temp);
        return;
    }

    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return;
    }

    prev->next = temp->next;
    mem_free(temp);
}

// Search for a node with the given data
Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Display the linked list
void list_display(Node** head) {
    list_display_range(head, NULL, NULL);
}

// Display a range of nodes in the linked list
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (head == NULL || *head == NULL) {
        printf("[]\n");
        return;
    }

    Node* current = (start_node == NULL) ? *head : start_node;
    printf("[");
    while (current != NULL && current != end_node) {
        printf("%d", current->data);
        if (current->next != NULL && current->next != end_node) {
            printf(", ");
        }
        current = current->next;
    }
    if (end_node != NULL) {
        printf("%s%d", (current != start_node) ? ", " : "", end_node->data);
    }
    printf("]\n");
}

// Count the number of nodes in the linked list
int list_count_nodes(Node** head) {
    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Clean up the linked list and free all memory
void list_cleanup(Node** head) {
    Node* current = *head;
    Node* next;

    while (current != NULL) {
        next = current->next;
        mem_free(current);
        current = next;
    }

    *head = NULL;
    mem_deinit();
}