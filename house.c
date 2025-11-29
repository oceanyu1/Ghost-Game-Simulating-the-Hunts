#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

void room_init(struct Room* room, const char* name, bool is_exit) {
    strncpy(room->name, name, MAX_ROOM_NAME);
    room->num_connected = 0;
    room->num_hunters = 0;
    room->ghost = NULL;
    room->evidence = 0;
    room->is_exit = is_exit;
    sem_init(&room->mutex, 0, 1);
}

void room_connect(struct Room* a, struct Room* b) {
    if (a->num_connected < MAX_CONNECTIONS && b->num_connected < MAX_CONNECTIONS) {
        a->connected[a->num_connected++] = b;
        b->connected[b->num_connected++] = a;
    }
}

void room_add_hunter(struct Room* room, struct Hunter* hunter) {
    if (room->num_hunters < MAX_ROOM_OCCUPANCY) {
        room->hunters[room->num_hunters++] = hunter;
    }
}

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