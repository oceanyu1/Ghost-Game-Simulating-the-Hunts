#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057
#define MAX_HUNTERS 4 

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading
};

// Implement here based on the requirements, should all be allocated to the House structure
struct Room {
    char name[MAX_ROOM_NAME];
    struct Room* connected[MAX_CONNECTIONS];
    int num_connected;
    struct Ghost* ghost; 
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY];
    int num_hunters;
    EvidenceByte evidence;
    sem_t mutex; 
    bool is_exit;
};

// Implement here based on the requirements, should be allocated to the House structure
struct Ghost {
    int id;
    enum GhostType type;
    struct Room* room;
    int boredom;
    bool running; 
    sem_t mutex;
};

// Can be either stack or heap allocated
struct House {
    struct Room rooms[MAX_ROOMS];
    int room_count;
    struct Room* starting_room; // Needed by house_populate_rooms, but can be adjusted to suit your needs.
    struct Hunter* hunters[MAX_HUNTERS]; 
    int hunter_count;
    struct Ghost* ghost;
    struct CaseFile case_file;
};

struct RoomNode {
    struct Room* room;
    struct RoomNode* next;
};

struct Hunter {
    int id;
    char name[MAX_HUNTER_NAME];
    struct Room* room;
    enum EvidenceType device;
    int fear;
    int boredom;
    struct CaseFile* case_file;
    struct RoomNode* path_stack; 
    bool running; 
    bool return_to_van; 
};

/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/

void room_init(struct Room* room, const char* name, bool is_exit);
void room_connect(struct Room* a, struct Room* b);
void room_add_hunter(struct Room* room, struct Hunter* hunter);
void room_remove_hunter(struct Room* room, struct Hunter* hunter);

struct Hunter* hunter_create(char* name, int id, struct Room* start_room, struct CaseFile* cf);
void hunter_destroy(struct Hunter* h);
void* hunter_thread(void* arg);

struct Ghost* ghost_create(int id, enum GhostType type, struct Room* start_room);
void ghost_destroy(struct Ghost* g);
void* ghost_thread(void* arg);

void stack_push(struct RoomNode** head, struct Room* room);
struct Room* stack_pop(struct RoomNode** head);
void stack_clean(struct RoomNode** head);

#endif // DEFS_H