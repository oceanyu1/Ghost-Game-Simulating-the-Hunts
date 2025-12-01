#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "helpers.h"
#include <time.h>

/**
 * @brief The main function for the Plasmophobia copycat game
 *
 * @return 0 if everything works
 */
int main() {
    
    // randomness
    srand(time(NULL)); 

    struct House house;
    house.hunter_count = 0;
    house.room_count = 0;
    house_populate_rooms(&house); 
    house.case_file.collected = 0;
    house.case_file.solved = false;
    
    // casefile semaphore
    sem_init(&house.case_file.mutex, 0, 1);

	// init ghost
    int ghost_start_idx = rand_int_threadsafe(1, house.room_count); 
    if (ghost_start_idx == 0) {
    	ghost_start_idx = 1; 
    }
    const enum GhostType* ghost_types;
    int num_ghosts = get_all_ghost_types(&ghost_types);
    enum GhostType g_type = ghost_types[rand_int_threadsafe(0, num_ghosts)];
    house.ghost = ghost_create(DEFAULT_GHOST_ID, g_type, &house.rooms[ghost_start_idx]);

	// init hunters
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
    
    // start ghost thread
    pthread_t ghost_identifier;
    pthread_create(&ghost_identifier, NULL, ghost_thread, house.ghost);

	// start hunter threads
    pthread_t hunter_identifiers[MAX_HUNTERS];
    for (int i = 0; i < house.hunter_count; i++) {
        pthread_create(&hunter_identifiers[i], NULL, hunter_thread, house.hunters[i]);
    }
    
    // stop hunter threads
    for (int i = 0; i < house.hunter_count; i++) {
        pthread_join(hunter_identifiers[i], NULL);
    }
    sem_wait(&house.ghost->mutex);
    house.ghost->running = false; 
    sem_post(&house.ghost->mutex);
    
    // stop ghost thread
    pthread_join(ghost_identifier, NULL);

	// results
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
    
    const char* ghost_guess = "N/A";
    
    // iterate through all ghosts for evidence match
    const enum GhostType* all_ghosts;
    int count = get_all_ghost_types(&all_ghosts);
    
    for (int i = 0; i < count; i++) {
        // if the collected evidence matches a ghost type
        if (house.case_file.collected == all_ghosts[i]) {
            ghost_guess = ghost_to_string(all_ghosts[i]);
            break;
        }
    }

    printf("Ghost Guess: %s\n", ghost_guess);
    
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
    
    // free memory
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