#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "helpers.h"

// stack functions
void stack_push(struct RoomNode** head, struct Room* room) {
    struct RoomNode* new_node = malloc(sizeof(struct RoomNode));
    if (new_node) {
        new_node->room = room;
        new_node->next = *head;
        *head = new_node;
    }
}

struct Room* stack_pop(struct RoomNode** head) {
    if (*head == NULL) return NULL;
    
    struct RoomNode* temp = *head;
    struct Room* room = temp->room;
    *head = temp->next;
    free(temp);
    return room;
}

void stack_clean(struct RoomNode** head) {
    while (*head != NULL) {
        stack_pop(head);
    }
}

// evidence helper functions

// check if the evidence byte has at least 3 bits set (1s)
bool evidence_has_three_unique(EvidenceByte mask) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if ((mask >> i) & 1) {
            count++;
        }
    }
    return count >= 3;
}