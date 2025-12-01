#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "helpers.h"

// stack functions

/**
 * @brief Pushes a room onto a linked list stack
 *
 * @param head Double pointer to the head of the stack
 * @param room Pointer to the Room to push onto the stack
 */
void stack_push(struct RoomNode** head, struct Room* room) {
    struct RoomNode* new_node = malloc(sizeof(struct RoomNode));
    if (new_node) {
        new_node->room = room;
        new_node->next = *head;
        *head = new_node;
    }
}

/**
 * @brief Pops a room from the front of the linked list stack
 *
 * @param head Double pointer to the head of the stack
 * @return Pointer to the Room popped from the stack/NULL (if empty)
 */
struct Room* stack_pop(struct RoomNode** head) {
    if (*head == NULL) return NULL;
    
    struct RoomNode* temp = *head;
    struct Room* room = temp->room;
    *head = temp->next;
    free(temp);
    return room;
}

/**
 * @brief Itererates through and frees all nodes in the linked list stack
 *
 * @param head Double pointer to the head of the stack
 */
void stack_clean(struct RoomNode** head) {
    while (*head != NULL) {
        stack_pop(head);
    }
}

// evidence helper functions

/**
 * @brief Check if the evidence byte has at least 3 bits set (1s)
 *
 * @param mask The EvidenceByte with the collected evidence
 * @return True if 3 or more bits are set, false otherwise
 */
bool evidence_has_three_unique(EvidenceByte mask) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if ((mask >> i) & 1) {
            count++;
        }
    }
    return count >= 3;
}