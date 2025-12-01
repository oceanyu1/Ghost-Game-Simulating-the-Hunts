#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // added this to fix weird strncpy warning
#include <unistd.h>
#include "defs.h"
#include "helpers.h"

/**
 * @brief Inits a new hunter struct
 *
 * @param name Hunter name
 * @param id Hunter ID
 * @param start_room Hunter starting room pointer
 * @param cf Pointer to the casefile for evidence
 * @return Pointer to the new Hunter struct
 */
struct Hunter* hunter_create(char* name, int id, struct Room* start_room, struct CaseFile* cf) {
    struct Hunter* h = malloc(sizeof(struct Hunter));
    strncpy(h->name, name, MAX_HUNTER_NAME);
    h->fear = 0;
    h->boredom = 0;
    h->id = id;
    h->room = start_room;
    h->case_file = cf;    
    h->path_stack = NULL;
    h->running = true;
    h->return_to_van = false;

    int dev_idx = rand_int_threadsafe(0, 7);
    switch(dev_idx) {
        case 0: h->device = EV_EMF; break;
        case 1: h->device = EV_ORBS; break;
        case 2: h->device = EV_RADIO; break;
        case 3: h->device = EV_TEMPERATURE; break;
        case 4: h->device = EV_FINGERPRINTS; break;
        case 5: h->device = EV_WRITING; break;
        case 6: h->device = EV_INFRARED; break;
    }
    
    log_hunter_init(h->id, start_room->name, h->name, h->device);
    return h;
}

/**
 * @brief Deletes hunter (frees memory)
 *
 * @param h Pointer to the Hunter
 */
void hunter_destroy(struct Hunter* h) {
    stack_clean(&h->path_stack);
    free(h);
}

/**
 * @brief The thread function for a hunter. Handles changing the fear or boredom, collecting evidence, 
 * moving from room to room, and checking if you won or lost
 *
 * @param arg Void pointer to the Hunter struct, casted to Hunter immediately
 * @return NULL after thread is over
 */
void* hunter_thread(void* arg) {
    struct Hunter* h = (struct Hunter*)arg;

    while (h->running) {
        struct Room* curr = h->room;

		// lock room to check for ghost
        sem_wait(&curr->mutex);
        
        // is ghost currently in room
        if (curr->ghost != NULL) {
            h->boredom = 0;
            h->fear += 1;
        } else {
            h->boredom += 1;
        }
        
        int current_boredom = h->boredom;
        int current_fear = h->fear;
        // unlock room
        sem_post(&curr->mutex);

		// r we in the van
        if (curr->is_exit) {
        	// clear path stack since we're back
            stack_clean(&h->path_stack);
            
            sem_wait(&h->case_file->mutex);
            if (h->case_file->solved) {
                sem_post(&h->case_file->mutex);
                h->running = false;
                log_exit(h->id, h->boredom, h->fear, curr->name, h->device, LR_EVIDENCE);
                break;
            }
            sem_post(&h->case_file->mutex);

            if (h->return_to_van) {
                 enum EvidenceType old_dev = h->device;
                 int dev_idx = rand_int_threadsafe(0, 7);
                 h->device = (1 << dev_idx);
                 log_swap(h->id, h->boredom, h->fear, old_dev, h->device);
                 h->return_to_van = false;
                 log_return_to_van(h->id, h->boredom, h->fear, curr->name, h->device, false);
            }
        }

		// r we either too scared or too bored
        if (current_fear >= HUNTER_FEAR_MAX) {
            h->running = false;
            sem_wait(&curr->mutex); 
            room_remove_hunter(curr, h);
            sem_post(&curr->mutex);
            log_exit(h->id, h->boredom, h->fear, curr->name, h->device, LR_AFRAID);
            break;
        }
        if (current_boredom >= ENTITY_BOREDOM_MAX) {
            h->running = false;
            sem_wait(&curr->mutex);
            room_remove_hunter(curr, h);
            sem_post(&curr->mutex);
            log_exit(h->id, h->boredom, h->fear, curr->name, h->device, LR_BORED);
            break;
        }

		// if we're not in the van and we're not too scared/bored
        if (!curr->is_exit) {
        	// lock room to check for evidence
            sem_wait(&curr->mutex);
            
            // check using bitwise if evidence/device is compatible
            if (curr->evidence & h->device) {
                curr->evidence &= ~h->device;
                sem_post(&curr->mutex);

                sem_wait(&h->case_file->mutex);
                h->case_file->collected |= h->device;
                if (evidence_has_three_unique(h->case_file->collected)) {
                     if (evidence_is_valid_ghost(h->case_file->collected)) {
                         h->case_file->solved = true; // we won woohoo	
                     }
                }
                sem_post(&h->case_file->mutex);

                log_evidence(h->id, h->boredom, h->fear, curr->name, h->device);
                
                h->boredom = 0; 
                h->return_to_van = true;
                log_return_to_van(h->id, h->boredom, h->fear, curr->name, h->device, true);
            } else {
                sem_post(&curr->mutex);
                int r = rand_int_threadsafe(0, 100);
                if (r < 10) { 
                    h->return_to_van = true;
                    log_return_to_van(h->id, h->boredom, h->fear, curr->name, h->device, true);
                }
            }
        }

        struct Room* next_room = NULL;

		// if we're returning to van
        if (h->return_to_van) {
             next_room = stack_pop(&h->path_stack);
        } else {
            int r = rand_int_threadsafe(0, curr->num_connected);
            next_room = curr->connected[r];
        }

		// go to next room
        if (next_room) {
        	// move safely with deadlock prevention
        	// compare memory addresses of both rooms and lock lower address first
            struct Room *first = (curr < next_room) ? curr : next_room;
            struct Room *second = (curr < next_room) ? next_room : curr;

            sem_wait(&first->mutex);
            sem_wait(&second->mutex);

            if (next_room->num_hunters < MAX_ROOM_OCCUPANCY) {
                room_remove_hunter(curr, h);
                room_add_hunter(next_room, h);
                h->room = next_room;
                
                if (!h->return_to_van) {
                	// push the room onto the breadcrumb stack
                    stack_push(&h->path_stack, curr);
                }
                
                log_move(h->id, h->boredom, h->fear, curr->name, next_room->name, h->device);
            } else {
                if (h->return_to_van) {
                	// pop next_room back onto the stack (since it was popped off before in line 163)
                    stack_push(&h->path_stack, next_room);
                }
            }

            sem_post(&second->mutex);
            sem_post(&first->mutex);
        }
        
        // added a little delay so you can see the hunter actions more clearly
        usleep(10000); 
    }
    return NULL;
}