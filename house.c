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