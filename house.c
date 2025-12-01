#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

/**
 * @brief Inits a room struct
 *
 * @param room Pointer to the Room struct
 * @param name Room name
 * @param is_exit Boolean for whether room is the exit
 */
void room_init(struct Room* room, const char* name, bool is_exit) {
    strncpy(room->name, name, MAX_ROOM_NAME);
    room->num_connected = 0;
    room->num_hunters = 0;
    room->ghost = NULL;
    room->evidence = 0;
    room->is_exit = is_exit;
    sem_init(&room->mutex, 0, 1);
}

/**
 * @brief Connects two rooms
 *
 * @param a Pointer to one room
 * @param b Pointer to the other room
 */
void room_connect(struct Room* a, struct Room* b) {
    if (a->num_connected < MAX_CONNECTIONS && b->num_connected < MAX_CONNECTIONS) {
        a->connected[a->num_connected++] = b;
        b->connected[b->num_connected++] = a;
    }
}

/**
 * @brief Adds a hunter to a specific room (as long as room isn't full)
 *
 * @param room Pointer to the Room
 * @param hunter Pointer to the Hunter
 */
void room_add_hunter(struct Room* room, struct Hunter* hunter) {
    if (room->num_hunters < MAX_ROOM_OCCUPANCY) {
        room->hunters[room->num_hunters++] = hunter;
    }
}

/**
 * @brief Removes a hunter from a specific room
 *
 * @param room Pointer to the Room
 * @param hunter Pointer to the Hunter
 */
void room_remove_hunter(struct Room* room, struct Hunter* hunter) {
    for (int i = 0; i < room->num_hunters; i++) {
        if (room->hunters[i] == hunter) {
            for (int j = i; j < room->num_hunters - 1; j++) {
                room->hunters[j] = room->hunters[j + 1];
            }
            room->hunters[room->num_hunters - 1] = NULL;
            room->num_hunters--;
            return;
        }
    }
}