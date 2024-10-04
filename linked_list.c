#include "linked_list.h"
#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>

void list_init(Node** head, size_t size) {
    *head = NULL;
    mem_init(size);
}

void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed for new node!\n");
        return;
    }

    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}

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
        new_node->next = *head;
        *head = new_node;
        return;
    }

    Node* temp = *head;
    while (temp->next != NULL && temp->next != next_node) {
        temp = temp->next;
    }

    if (temp->next == next_node) {
        new_node->next = next_node;
        temp->next = new_node;
    } else {
        printf("Next node not found in the list.\n");
        mem_free(new_node);
    }
}

void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        printf("List is empty. Cannot delete.\n");
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
        printf("Node with data %d not found.\n", data);
        return;
    }

    prev->next = temp->next;
    mem_free(temp);
}

Node* list_search(Node** head, uint16_t data) {
    Node* temp = *head;

    while (temp != NULL) {
        if (temp->data == data) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
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

    if (start_node == NULL) {
        start_node = *head;
    }

    while (temp != NULL && temp != start_node) {
        temp = temp->next;
    }

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
    *head = NULL;
    mem_deinit();
}