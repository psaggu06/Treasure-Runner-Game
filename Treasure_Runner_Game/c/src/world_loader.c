#include "world_loader.h"
#include <stdlib.h>
#include <string.h>
#include "datagen.h"
#include "graph.h"
#include "types.h"
#include "room.h"

static int compare_rooms(const void *a, const void *b){
    const Room *ra = (const Room*)a;
    const Room *rb = (const Room*)b; 
    if (ra->id < rb->id) return -1;
    if (ra->id > rb->id) return 1;
    return 0; 
}//End of compare_rooms

static void destroy_room(void *r){
    room_destroy((Room*)r); //call room_destroy to free room
}//End of destroy_room

static Status portal_loading(Room *r, DG_Room *dgr){
    if (dgr->portal_count > 0) {
        Portal *p = malloc(dgr->portal_count * sizeof(Portal)); //allocate memory for portal
        if (p == NULL){ 
            stop_datagen(); //stop datagen
            return NO_MEMORY; //return memory allocation error
        }//end of if statement 
        for (int i = 0; i < dgr->portal_count; i++) { 
            //set each portal attribute to the datagen portal attributes and set the portal name to NULL
            p[i].name = NULL;
            p[i].id = dgr->portals[i].id;
            p[i].target_room_id = dgr->portals[i].neighbor_id;
            p[i].x = dgr->portals[i].x;
            p[i].y = dgr->portals[i].y;
        }//end of for loop
        Status s = room_set_portals(r, p, dgr->portal_count); //call room_set_portals and check status
        if (s != OK) {
            free(p); //free portal if unsuccesful
            stop_datagen(); //stop datagen
            return s; //return the status to indicate the type of error
        }//end of if statement
    }//end of if statement
    return OK; //return OK to indicate success
}//End of portal_loading

static Status treasure_loading(Room *r, DG_Room *dgr){
    if (dgr->treasure_count > 0) {
        Treasure *t = malloc(dgr->treasure_count * sizeof(Treasure)); //allocate memory for treasure 
        if (t == NULL) {
            stop_datagen(); //stop datagen
            return NO_MEMORY; //return NO_MEMORY error
        }//end of if statement
        for (int i = 0; i < dgr->treasure_count; i++) {
            //set each treasure attribute to the datagen treasure attributes
            t[i].id = dgr->treasures[i].global_id;
            t[i].x = dgr->treasures[i].x;
            t[i].y = dgr->treasures[i].y;
            t[i].initial_x = dgr->treasures[i].x;
            t[i].initial_y = dgr->treasures[i].y;
            t[i].starting_room_id = dgr->id;
            t[i].collected = false;
            t[i].name = malloc(strlen(dgr->treasures[i].name) + 1); //allocate memory for treasure name
            if(t[i].name == NULL) {
                for (int j = 0; j < i; j++) {
                    free(t[j].name); //free each treasure name if a certain treasure name is null
                }//end of inner for loop
                free(t); //free treasure itself
                stop_datagen(); //stop datagen
                return NO_MEMORY; //return NO_MEMORY error
            }//end of inner if statement
            strcpy(t[i].name, dgr->treasures[i].name); //copy the treadure name into the datagen room treasure name
        }//end of for loop
            Status s = room_set_treasures(r, t, dgr->treasure_count); //call room_set_treasures and check status
        if (s != OK) {
            free(t); //free treasure if unsuccesful
            stop_datagen(); //stop datagen
            return s; //return status 
        }//end of if statement
    }//end of if statement
    return OK; //return OK to indicate success
}//End of treasure_loading

static Status pushable_loading(Room *r, DG_Room *dgr){
    if (dgr->pushable_count > 0) {
        Pushable *psh = malloc(dgr->pushable_count * sizeof(Pushable)); //allocate memory for pushable
        if (psh == NULL) {
            stop_datagen(); //stop datagen
            return NO_MEMORY; //return NO_MEMORY error
        }//end of if statement
        for (int i = 0; i < dgr->pushable_count; i++) {
            //set each pushable attribute to the datagen pushable attributes
            psh[i].id = dgr->pushables[i].id;
            psh[i].x = dgr->pushables[i].x; 
            psh[i].initial_x = dgr->pushables[i].x;
            psh[i].y = dgr->pushables[i].y;
            psh[i].initial_y = dgr->pushables[i].y;
            psh[i].name = malloc(strlen(dgr->pushables[i].name) + 1); //allocate memory for pushable name
            if(psh[i].name == NULL) {
                for (int j = 0; j < i; j++) {
                    free(psh[j].name); //free each pushable name if a certain pushable name is null
                }//end of inner for loop
                free(psh); //free pushable itself
                stop_datagen(); //stop datagen
                return NO_MEMORY; //return NO_MEMORY error
            }//end of inner if statement
            strcpy(psh[i].name, dgr->pushables[i].name); //copy the pushable name into the datagen room pushable name
        }//end of for loop
        r->pushables = psh;
        r->pushable_count = dgr->pushable_count;
    }//end of if statement
    return OK; //return OK to indicate success
}//End of pushable_loading

static void connect_rooms_in_graph(Graph *g){
    //initialize variables
    int target_id = -1;
    int payloads = 0;
    const void * const *payloads_out = NULL;
    Room key = {0};
    if (graph_get_all_payloads(g, &payloads_out, &payloads) == GRAPH_STATUS_OK) { //call graph_get_all_payloads and verify success
        for (int i = 0; i < payloads; i++) {
            Room *r = (Room*)payloads_out[i]; //set room equal to each payload with a cast
            for (int j = 0; j < r->portal_count; j++) {
                target_id = r->portals[j].target_room_id; //set the target room id to the room portals target room id for each
                if (target_id != -1) {
                    key.id = target_id; //set the key's id to the target room id
                    Room *t = (Room*)graph_get_payload(g, &key); //create the target room and set it to graph_get_payload(), calling it with the target room id
                    graph_connect(g, r, t); //connect the room and target room in the graph afterwards
                }//end of inner if statement
            }//end of inner for loop
        }//end of outer for loop
    }//end of outer if statement
}//End of connect_rooms_in_graph

Status loader_load_world(const char *config_file, Graph **graph_out, Room **first_room_out, int  *num_rooms_out, Charset *charset_out){
    if (config_file == NULL || graph_out == NULL || first_room_out == NULL || num_rooms_out == NULL || charset_out == NULL) {
        return INVALID_ARGUMENT; //returns invalid_argument if any of the parameters are NULL
    }//end of if statement 

    //initialize variables...
    int count = 0;
    bool *floor_grid = NULL;
    Room *r = NULL;
    Room *r1 = NULL;
    DG_Room dgr = {0};
    Graph *g = NULL;
    Status s = NULL_POINTER;
    size_t grid_dimensions = 0;
    *graph_out = NULL;
    *first_room_out = NULL;
    *num_rooms_out = 0;
    
    int dgs = start_datagen(config_file); //call start_datagen() with .ini file
     //return the appropriate error if data gen status is not successful
    if (dgs == DG_ERR_CONFIG) {
        return WL_ERR_CONFIG; //returns configuration error if configuration path is invalid
    }//end of if statement
    if (dgs == DG_ERR_OOM) {
        return NO_MEMORY; //returns memory error if memory allocation fails
    }//end of if statement
    if (dgs != DG_OK) {
        return WL_ERR_DATAGEN; //returns datagen error if datagen fails
    }//end of if statement
    const DG_Charset *dg_cs = dg_get_charset(); //call dg_get_charset()
    if (dg_cs == NULL) {
        stop_datagen(); //stop datagen if datagen charset is NULL
        return WL_ERR_DATAGEN; //return datagen error
    }//end of if statement
    //Set every attribute of charset_out to datagen charset
    charset_out->wall = dg_cs->wall;
    charset_out->floor = dg_cs->floor;
    charset_out->player = dg_cs->player;
    charset_out->treasure = dg_cs->treasure;
    charset_out->portal = dg_cs->portal;
    charset_out->pushable = dg_cs->pushable;
    if (graph_create(compare_rooms, destroy_room, &g) != GRAPH_STATUS_OK) {  //call graph_create and verify success
        stop_datagen(); //stop datgen if graph status is not succesful 
        return NO_MEMORY; //return NO_MEMORY error
    }//end of if statement

    while (has_more_rooms()) { //while loop continues to run until no more additional rooms exist
        dgr = get_next_room(); //call get_next_room to get the next room in the file
        r = room_create(dgr.id, NULL, dgr.width, dgr.height); //create room with given parameters 
        if (r == NULL) { 
            graph_destroy(g); //destroy graph if room is NULL
            stop_datagen(); //stop datagen
            return NULL_POINTER; //return NULL error
        }//end of if statement 

        grid_dimensions = (size_t)dgr.width * (size_t)dgr.height; //calculate grid size by multiplying data gen room height and width with size_t cast

        floor_grid = malloc(grid_dimensions * sizeof(bool)); //allocate memory for floor grid
        if (floor_grid == NULL) {
            room_destroy(r); //destroy room if floor grid is NULL
            graph_destroy(g); //destroy graph
            stop_datagen(); //stop datagen
            return NO_MEMORY; //return memory error 
        }//end of if statement
        for (size_t i = 0; i < grid_dimensions; i++) {
            floor_grid[i] = dgr.floor_grid[i]; //set each floor grid to the datagen room floor grid until it reaches the max size of the grid
        }//end of for loop

        s = room_set_floor_grid(r, floor_grid); //call room_set_floor_grid
        if (s != OK) {
            free(floor_grid); //free floor grid memory if status indicates failure
            room_destroy(r); //destroy room 
            graph_destroy(g); //destroy graph
            stop_datagen(); //stop datagen
            return s; //return the status to indicate error
        }//end of if statement
        portal_loading(r, &dgr); //call portal loading function to load portals for the room
        treasure_loading(r, &dgr); //call treasure loading function to load treasures for the room
        pushable_loading(r, &dgr); //call pushable loading function to load pushables for the room
        graph_insert(g, r); //insert the room into the graph
        if (r1 == NULL) {
            r1 = r; //set the first room to the current first room if NULL
        }//end of if statement
        count++; //increment count
    }//end of while loop 
    connect_rooms_in_graph(g); //call connect_rooms_in_graph to connect all rooms in graph accordingly
    stop_datagen(); //stop datagen once succesful
    *graph_out = g;  //set graph_out to graph
    *first_room_out = r1; //set first_room_out to first room
    *num_rooms_out = count; //set num_rooms_out to count
    return OK; //return OK to indicate successful world load
}//End of loader_load_world