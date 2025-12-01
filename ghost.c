#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "defs.h"
#include "helpers.h"

/**
 * @brief Inits a new ghost struct
 *
 * @param id Ghost ID
 * @param type Type of ghost
 * @param start_room Ghost starting room pointer
 * @return Pointer to the new Ghost struct
 */
struct Ghost* ghost_create(int id, enum GhostType type, struct Room* start_room) {
    struct Ghost* g = malloc(sizeof(struct Ghost));
    g->id = id;
    g->type = type;
    g->room = start_room;
    g->boredom = 0;
    g->running = true;
    sem_init(&g->mutex, 0, 1);
    
    g->room->ghost = g;
    log_ghost_init(id, start_room->name, type);
    return g;
}

/**
 * @brief Deletes ghost (frees memory)
 *
 * @param g Pointer to the Ghost
 */
void ghost_destroy(struct Ghost* g) {
    sem_destroy(&g->mutex); 
    free(g);
}

/**
 * @brief The thread function for a ghost. Handles changing the boredom, dropping evidence, 
 * moving from room to room, and leaving when boredom goes over the limit
 *
 * @param arg Void pointer to the Ghost struct, casted to Ghost immediately
 * @return NULL after thread is over
 */
void* ghost_thread(void* arg) {
    struct Ghost* g = (struct Ghost*)arg;

    while (1) {
    	// first check if we should keep running
    	sem_wait(&g->mutex);
        bool cont = g->running;
        sem_post(&g->mutex);
        
        // bbreak out of loop if we're done
        if (!cont) {
        	break;
        }
        struct Room* curr = g->room;
        	
        // lock room (in case hunter is entering/leaving)
        sem_wait(&curr->mutex);
        
        if (curr->num_hunters > 0) {
            g->boredom = 0;
            sem_post(&curr->mutex);
        } else {
            g->boredom++;
            sem_post(&curr->mutex);
        }

		// if bored
        if (g->boredom >= ENTITY_BOREDOM_MAX) {
            sem_wait(&g->mutex);
            g->running = false;
            sem_post(&g->mutex);

            sem_wait(&curr->mutex);
            curr->ghost = NULL; 
            sem_post(&curr->mutex);
            log_ghost_exit(g->id, g->boredom, curr->name);
            break;
        }

        int action = rand_int_threadsafe(0, 3); // 0 idle, 1 haunt, 2 move

        if (action == 0) { 
        	// do nothing
            log_ghost_idle(g->id, g->boredom, curr->name); 
        } 
        else if (action == 1) {
        	// haunt
            int bits[3];
            int count = 0;
            for (int i = 0; i < 7; i++) {
                if (g->type & (1 << i)) {
                    bits[count++] = (1 << i);
                }
            }
            int choice = bits[rand_int_threadsafe(0, 3)];
            
            sem_wait(&curr->mutex);
            curr->evidence |= choice;
            sem_post(&curr->mutex);
            
            log_ghost_evidence(g->id, g->boredom, curr->name, choice);
        }
        else if (action == 2) {
        	//move
            sem_wait(&curr->mutex);
            bool hunter_present = (curr->num_hunters > 0);
            sem_post(&curr->mutex);

            if (!hunter_present) {
                int r = rand_int_threadsafe(0, curr->num_connected);
                struct Room* next = curr->connected[r];
                
                // move safely with deadlock prevention
                struct Room *first = (curr < next) ? curr : next;
                struct Room *second = (curr < next) ? next : curr;

                sem_wait(&first->mutex);
                sem_wait(&second->mutex);
                
                curr->ghost = NULL;
                next->ghost = g;
                g->room = next;
                
                sem_post(&second->mutex);
                sem_post(&first->mutex);

                log_ghost_move(g->id, g->boredom, curr->name, next->name);
            }
        }
        
        usleep(1000); 
    }
    return NULL;
}