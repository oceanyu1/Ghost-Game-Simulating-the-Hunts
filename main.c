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
    if (ghost_start_idx == 0) {
    	ghost_start_idx = 1; 
    }
    const enum GhostType* ghost_types;
    int num_ghosts = get_all_ghost_types(&ghost_types);
    enum GhostType g_type = ghost_types[rand_int_threadsafe(0, num_ghosts)];
    house.ghost = ghost_create(DEFAULT_GHOST_ID, g_type, &house.rooms[ghost_start_idx]);

    char name_buffer[MAX_HUNTER_NAME];
    printf("Please enter hunter names ('done' to cancel):\n");
    while (house.hunter_count < MAX_HUNTERS) {
        printf("Name: ");
        scanf("%63s", name_buffer);
        if (strcmp(name_buffer, "done") == 0) {
        	break;
        }
        
        int h_id = house.hunter_count + 1; 

        struct Hunter* h = hunter_create(name_buffer, h_id, house.starting_room, &house.case_file);
        
        house.hunters[house.hunter_count++] = h;
        room_add_hunter(house.starting_room, h);
    }
    
    pthread_t ghost_identifier;
    pthread_create(&ghost_identifier, NULL, ghost_thread, house.ghost);

    pthread_t hunter_identifiers[MAX_HUNTERS];
    for (int i = 0; i < house.hunter_count; i++) {
        pthread_create(&hunter_identifiers[i], NULL, hunter_thread, house.hunters[i]);
    }
    for (int i = 0; i < house.hunter_count; i++) {
        pthread_join(hunter_identifiers[i], NULL);
    }
    sem_wait(&house.ghost->mutex);
    house.ghost->running = false; 
    sem_post(&house.ghost->mutex);
    pthread_join(ghost_identifier, NULL);

    printf("\n--- Simulation Results ---\n");
    printf("Type of Ghost: %s\n", ghost_to_string(house.ghost->type));
    
    printf("Evidence Collected: ");
    const enum EvidenceType* ev_list;
    int ev_count = get_all_evidence_types(&ev_list);
    for (int i = 0; i < ev_count; i++) {
        if (house.case_file.collected & ev_list[i]) {
            printf("%s ", evidence_to_string(ev_list[i]));
        }
    }
    printf("\n");
    
    bool result = house.case_file.solved;
    if (result) {
    	printf("Result: Hunters WON! :D\n");
    } else {
    	printf("Result: Hunters FAILED. :(\n");
	}
    for (int i = 0; i < house.hunter_count; i++) {
        struct Hunter* h = house.hunters[i];
        printf("Hunter %s --- Fear %d --- Boredom %d\n", h->name, h->fear, h->boredom);
    }
    ghost_destroy(house.ghost);
    for (int i = 0; i < house.hunter_count; i++) {
        hunter_destroy(house.hunters[i]);
    }
    sem_destroy(&house.case_file.mutex);
    for (int i = 0; i < house.room_count; i++) {
        sem_destroy(&house.rooms[i].mutex);
    }

    return 0;
}