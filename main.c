#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "helpers.h"
#include <time.h>

int main() {
    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */
    
    srand(time(NULL)); 

    struct House house;
    house.hunter_count = 0;
    house.room_count = 0;
    house_populate_rooms(&house); 

    house.case_file.collected = 0;
    house.case_file.solved = false;
    sem_init(&house.case_file.mutex, 0, 1);

    int ghost_start_idx = rand_int_threadsafe(1, house.room_count); 
    if (ghost_start_idx == 0) ghost_start_idx = 1; 
    
    const enum GhostType* ghost_types;
    int num_ghosts = get_all_ghost_types(&ghost_types);
    enum GhostType g_type = ghost_types[rand_int_threadsafe(0, num_ghosts)];
    
    house.ghost = ghost_create(DEFAULT_GHOST_ID, g_type, &house.rooms[ghost_start_idx]);

    char name_buffer[MAX_HUNTER_NAME];
    printf("Please enter hunter names ('done' to cancel):\n");
    while (house.hunter_count < MAX_HUNTERS) {
        printf("Name: ");
        scanf("%63s", name_buffer);
        if (strcmp(name_buffer, "done") == 0) break;
        
        int h_id = house.hunter_count + 1; 

        struct Hunter* h = hunter_create(name_buffer, h_id, house.starting_room, &house.case_file);
        
        house.hunters[house.hunter_count++] = h;
        room_add_hunter(house.starting_room, h);
    }

    printf("\n--- Simulation Results ---\n");
    
    printf("Evidence Collected: ");

    return 0;
}