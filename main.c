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

    int ghost_start_idx = rand_int_threadsafe(1, house.room_count); 
    if (ghost_start_idx == 0) ghost_start_idx = 1;
    
    char name_buffer[MAX_HUNTER_NAME];
    printf("Please enter hunter names ('done' to cancel):\n");
    while (house.hunter_count < MAX_HUNTERS) {
    
    }

    printf("\n--- Simulation Results ---\n");
    
    printf("Evidence Collected: ");

    return 0;
}