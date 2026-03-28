#include "game_engine.h"
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "world_loader.h"
#include "graph.h"
#include "room.h"
#include "player.h"

//prototypes for extra functions that are used for extended features
Status game_engine_access_portal(GameEngine *eng);
Status game_engine_get_treasure_total(const GameEngine *eng, int *total_out);
Status game_engine_get_adjacency_matrix(const GameEngine *eng, int **matrix_out);

static Status check_for_pushables(Room *r, int psh_idx, Direction dir, int new_x, int new_y){
    int push_x = new_x; //set the push x coordinate to the new x 
    int push_y = new_y; //set the push y coordinate to the new y
    if (dir == DIR_NORTH){
        push_y--; //up
    } else if (dir == DIR_WEST){
        push_x--; //left
    } else if (dir == DIR_SOUTH){
        push_y++; //down
    } else if (dir == DIR_EAST){
        push_x++; //right
    }//end of if-else statements
    if (room_get_treasure_at(r, push_x, push_y) != -1) {
        return ROOM_IMPASSABLE; //return ROOM_IMPASSABLE if there is a treasure behind the pushable
    }//end of if statement
    //check if there is a portal at the pushable's new coordinates 
    if (r->portals != NULL) { //check if portals exist in the room to avoid unnecessary for loop
        for (int i = 0; i < r->portal_count; i++) { 
            if (r->portals[i].x == push_x && r->portals[i].y == push_y) { //if there is a portal at the pushable's new coordinates..
                return ROOM_IMPASSABLE; //return ROOM_IMPASSABLE if there is a portal at the pushable
            }//end of if statement
        }//end of for loop
    }//end of if statement
    if (room_try_push(r, psh_idx, dir) != OK) {
        return ROOM_IMPASSABLE; //return ROOM_IMPASSABLE if unable to push the pushable
    }//end of inner if statement
    return OK; //return OK to indicate success
}//End of check_for_pushables

static Status check_for_portals(GameEngine *eng, int dest_of_portal){
    int target_x = 0; //initialize target x coordinate
    int target_y = 0; //initialize target y coordinate
    int treasure_id = -1; //initialize treasure id to -1
    Treasure *t = NULL; //initialize treasure pointer for checking if there is a treasure at the portal destination
    Room target_k = {0}; //initialize room key for retrieving the target room from the graph
    target_k.id = dest_of_portal; //set target key id to the portal location
    Room *target_r = (Room*)graph_get_payload(eng->graph, &target_k); //set the target room to the portal location in the engine graph by calling graph_get_payload
    if (room_get_start_position(target_r, &target_x, &target_y) != OK){ //call room_get_start_position with target dimensions if target room is not walkable 
        return GE_NO_SUCH_ROOM; //return GAME_ENGINE_NO_SUCH_ROOM if room doesnt exist
    }//end of inner if statement
    if (player_move_to_room(eng->player, dest_of_portal) != OK){ //call player_move_to_room to move engine player to the room of portal
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unsuccesful
    }//end of inner if statement
    if (player_set_position(eng->player, target_x, target_y) != OK) { //call player_set_position to set engine player's position to the target dimensions
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unsuccesful
    }//end of inner if statement 
    treasure_id = room_get_treasure_at(target_r, target_x, target_y);
    if (treasure_id != -1) {
        if (room_pick_up_treasure(target_r, treasure_id, &t) == OK && t != NULL) { //call room_pick_up_treasure at target room 
            t->collected = false; //set the treasure to uncollected since player is not on the tile yet, so it can be collected by player_try_collect
            player_try_collect(eng->player, t); //call player_try_collect to collect treasure 
        }//end of inner statement
    }//end of inner if statement
    return OK; //return OK to indicate success
}//End of check_for_portals

Status game_engine_create(const char *config_file_path, GameEngine **engine_out){
    if (config_file_path == NULL || engine_out == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUEMENT if file path or engine out are NULL
    }//end of if statement
    //initialize variables
    int initial_x = 0;
    int initial_y = 0;
    Player *p = NULL;
    //initialize variables for world loader
    Graph *g = NULL;
    Room *r = NULL;
    int count = 0;
    Charset cs;

    GameEngine *eng = malloc(sizeof(GameEngine)); //allocate memory for game engine
    if (eng == NULL){
        return NO_MEMORY; //return NO_MEMORY if malloc fails
    } //end of if statement
    //initialize each attribute for the game engine
    eng->graph = NULL;
    eng->player = NULL;
    eng->room_count = 0;
    eng->initial_room_id = 0;
    eng->initial_player_x = 0;
    eng->initial_player_y = 0;

    Status s = loader_load_world(config_file_path, &g, &r, &count, &cs); //call loader_load_world and verify success

    if (s != OK) {
        free(eng); //free engine if status indicate error
        return s; //return status to tell us the type of error
    }//end of if statement

    //set each engine attribute to the current graph, room count, charset, and the initial room id received from loader_load_world
    eng->graph = g;
    eng->room_count = count;
    eng->charset = cs;
    eng->initial_room_id = r->id;
    
    if (room_get_start_position(r, &initial_x, &initial_y) != OK) {
        //set the initial X and Y positions to 1 if unable to receive starting position
        initial_x = 1;
        initial_y = 1;
    }//end of if statement

    //set the engines initial player positions to the current initial positons 
    eng->initial_player_x = initial_x;
    eng->initial_player_y = initial_y;

    s = player_create(r->id, initial_x, initial_y, &p); //create player and check status
    if (s != OK) {
        free(eng); //free engine if unsuccessful
        return s; //return the status to indicate type of error
    }//end of if statement

    eng->player = p; //set the engine player to the current player
    *engine_out = eng; //set the engine pointer to the engine
    return OK; //return OK to indicate function success
}//End of game_enginge_create

void game_engine_destroy(GameEngine *eng){
    if (eng == NULL){
        return; //return if engine is NULL
    }//end of if statement
    if(eng->player != NULL){
        player_destroy(eng->player); //destroy and free engine player
    }//end of if statement
    if(eng->graph != NULL){
        graph_destroy(eng->graph); //destroy and free engine graph
    }//end of if statement
    free(eng); //free the engine itself
}//End of game_engine_destroy

const Player *game_engine_get_player(const GameEngine *eng){
    if (eng == NULL) {
        return NULL; //return NULL if engine is NULL
    }//end of if statement
    return eng->player; //return the engine player
}//End of game_engine_get_player

Status game_engine_move_player(GameEngine *eng, Direction dir){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine is NULL
    }//end of if statement
    //initialize variables
    int cx = 0;
    int cy = 0;
    int new_x = 0;
    int new_y = 0;
    int treasure_id = -1;
    int id = -1;
    int pushable_idx = -1;
    Treasure *t = NULL;
    Room key = {0};

    id = player_get_room(eng->player); //call player_get_room for engine player
    key.id = id; //set key.id to the id of the player room
    Room *r = (Room*)graph_get_payload(eng->graph, &key); //set room to the id of the room key 
    if (r == NULL){
        return INTERNAL_ERROR; //return INTERNAL_ERROR if room is NULL
    }//end of if statement

    if (player_get_position(eng->player, &cx, &cy) != OK) {  //call player_get_position and check status 
        return INTERNAL_ERROR; //return INTERNAL_ERROR if failsure to get pos
    }//end of if statement 
    //set the new X and Y positions to the current X and Y values received from the player's position
    new_x = cx;
    new_y = cy;

    //increment or decrement X or Y depending on the direction retrieved from the Player's position
    if (dir == DIR_NORTH){
        new_y--; //up
    } else if (dir == DIR_WEST){
        new_x--; //left
    } else if (dir == DIR_SOUTH){
        new_y++; //down
    } else if (dir == DIR_EAST){
        new_x++; //right
    } else {
        return INVALID_ARGUMENT;
    }//end of if-else statements

    treasure_id = room_get_treasure_at(r, new_x, new_y);  //call room_pick_up_treasure at current room 
    if (treasure_id != -1) {
        if (room_pick_up_treasure(r, treasure_id, &t) == OK && t != NULL) {
            t->collected = false; //set the treasure to uncollected since player is not on the tile yet, so it can be collected by player_try_collect
            player_try_collect(eng->player, t);//call player_try_collect to collect treasure
        }//end of inner statement
        return OK; //return OK if treasure is picked up and collected 
    }//end of inner if statement

    if (room_has_pushable_at(r, new_x, new_y, &pushable_idx)) {
        Status s = check_for_pushables(r, pushable_idx, dir, new_x, new_y); //call check_for_pushables to check if player can push the pushable at the new coordinates in the direction they are moving
        if (s != OK) {
            return s; //return the status from check_for_pushables if it is not OK
        }//end of if statement
    }//end of if statement
    if (!room_is_walkable(r, new_x, new_y)){ //check if player can move after checking for pushable
        return ROOM_IMPASSABLE; //return ROOM_IMPASSABLE error if not walkable
    }//end of if statement

    //check if portal is at new coordinates
    // dest_of_portal = room_get_portal_destination(r, new_x, new_y); //call room_get_portal_destination 
    // if (dest_of_portal != -1) { //if exists, find the destination room in the graph using ID
    //     return check_for_portals(eng, dest_of_portal); //call check_for_portals to check if player can go through the portal
    // }//end of if statement
    //if portal does not exist at tile move within current room
    if (player_set_position(eng->player, new_x, new_y) != OK) { //call player_set_position to set engine player's position to the current room dimensions
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unsuccesful
    }//end of if statement
    return OK; //return OK to indicate
}//End of game_engine_move_player

Status game_engine_get_room_count(const GameEngine *eng, int *count_out){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine or count out are NULL
    }//end of if statement
    if (count_out == NULL) {
        return NULL_POINTER; //return NULL_POINTER if count out is NULL
    }//end of if statement
    *count_out = eng->room_count; //set the count pointer to the engine room count
    return OK;
}//End of game_engine_get_room_count


Status game_engine_get_room_dimensions(const GameEngine *eng, int *width_out, int *height_out){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine, width out, or height out are NULL
    }//end of if statement
    if (width_out == NULL || height_out == NULL) {
        return NULL_POINTER; //return NULL_POINTER if width out or height out are NULL
    }//end of if statement

    //initialize variables
    int id = player_get_room(eng->player); //set the id to the room the player is currently in
    Room key;
    key.id = id;
    Room *r = (Room*)graph_get_payload(eng->graph, &key); //call graph_get_payload to set room to the id of the room key  
    *height_out = room_get_height(r); //call room_get_height to get height of the room
    *width_out = room_get_width(r); //call room_get_width to get width of the room
    return OK;
}//End of game_engine_get_room_dimensions

Status game_engine_reset(GameEngine *eng){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine is NULL
    }//end of if statement
    //initialize variables
    int *ids = NULL;
    int count = 0;
    Room k = {0};

    if (player_reset_to_start(eng->player, eng->initial_room_id, eng->initial_player_x, eng->initial_player_y) != OK) { //call player_reset_to_start
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unable to reset player
    }//end of if statement
    
    if (game_engine_get_room_ids(eng, &ids, &count) != OK) { //call game_engine_get_room_ids to retrieve all room ids in the world
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unable to retrieve room ids
    }//end of if statement

    for (int i = 0; i < count; i++) { //for each room id retrieved, call room_reset to reset the room to its initial state
        k.id = ids[i]; //set key id to the current id
        Room *r = (Room*)graph_get_payload(eng->graph, &k); //set room to the id of the room key  
        if (r == NULL) {
            free(ids); //free ids if unable to reset a room
            return INTERNAL_ERROR; //return INTERNAL_ERROR if unable to reset a room
        }//end of if statement
        for (int j = 0; j < r->treasure_count; j++) {
            r->treasures[j].collected = false; //set each treasure in the room to uncollected
            r->treasures[j].x = r->treasures[j].initial_x; //set each treasure in the room to its intial x position
            r->treasures[j].y = r->treasures[j].initial_y; //set each treasure in the room to its initial y position
        }//end of inner for loop
        for (int j = 0; j < r->pushable_count; j++) {
            r->pushables[j].x = r->pushables[j].initial_x; //set each pushable in the room to its initial x position
            r->pushables[j].y = r->pushables[j].initial_y; //set each pushable in the room to its initial y position
        }//end of inner for loop
    }//end of for loop
    free(ids); //free ids after resetting all rooms
    return OK;
}//end of game_engine_reset

Status game_engine_render_current_room(const GameEngine *eng, char **str_out){
    //initialize variables
    if (eng == NULL || str_out == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine or string output pointer are NULL
    }//end of if statement
    int px = 0;
    int py = 0;
    int push_x = 0;
    int push_y = 0; 
    int width = 0;
    int height = 0;
    int id = player_get_room(eng->player); //call player_get_room to retrieve the room the player is currently in
    char *buffer = NULL;
    char * result = NULL;
    char *temp = NULL;
    Room key;
    size_t buffer_s = 0;

    key.id = id;
    Room *r = (Room*)graph_get_payload(eng->graph, &key); //set room to the id of the room key  

    if (player_get_position(eng->player, &px, &py) != OK) { //call player_get_position 
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unable to retrieve player's position
    }//end of if statement 

    height = room_get_height(r); //call room_get_height to get height of the room
    width = room_get_width(r);  //call room_get_width to get width of the room
    buffer_s = (size_t)height * (size_t)width; //set the buffer size equal to the height * width with size_t cast
    buffer = malloc(sizeof(char) * buffer_s); //allocate memory for buffer
    if (buffer == NULL){
        return NO_MEMORY; //return NO_MEMORY error if allocation fails
    }//end of if statement

    if (room_render(r, &eng->charset, buffer, width, height) != OK){ //call room_render 
        free(buffer); //free buffer if unsuccessful
        return INTERNAL_ERROR; //return INTERNAL_ERROR 
    }//end of if statement

    for (int i = 0; i < r->pushable_count; i++) {
        push_x = r->pushables[i].x; //set x coordinate of current pushable
        push_y = r->pushables[i].y; //set y coordinate of current pushable
        if (push_x >= 0 && push_y >= 0 && push_x < width && push_y < height) { //check if coordintes are greater than zero and less than width and height
            buffer[push_y * width + push_x] = eng->charset.pushable; //set the buffer at the calculated index to the engine charset for current pushable
        }//end of if statement
    }//end of for loop

    if (px >= 0 && py >= 0 && px < width && py < height){
        buffer[py * width + px] = eng->charset.player; //set the buffer at the calculated index to the engine charset for player
    }//end of if statement

    result = malloc(sizeof(char) * (buffer_s + height + 1)); //allocate memory for the result
    if (result == NULL) {
        free(buffer); //free buffer if allocation fails
        return NO_MEMORY; //return NO_MEMORY error
    }//end of if statement

    temp = result; //set the temp pointer to the result

    //converts 1D character buffer (no linefeeds) into multi-line string (with linefeeds)
    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) {
            temp[j] = buffer[i * width + j]; //set each temp element to the buffer at the x and y position
        }//end of inner for loop
        temp += width; //add temp to width and set it to that result
        *temp++ = '\n'; //store newline at current index then increment
    }//end of outer for loop
    *temp = '\0'; //set temp pointer to null character

    free(buffer); //free buffer
    *str_out = result; //set the string pointer to the result
    return OK;
}//End of game_engine_render_current_room

Status game_engine_render_room(const GameEngine *eng, int room_id, char **str_out){
    if (eng == NULL || str_out == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine or string output pointer are NULL
    }//end of if statement
    //initialize variables
    int px = 0;
    int py = 0;
    int push_x = 0;
    int push_y = 0;
    int width = 0;
    int height = 0;
    char *buffer = NULL;
    char * result = NULL;
    char *temp = NULL;
    Room key;
    size_t buffer_s = 0;

    key.id = room_id;
    Room *r = (Room*)graph_get_payload(eng->graph, &key); //set room to the id of the room key  
    if (r == NULL) {
        return GE_NO_SUCH_ROOM; //return GAME_ENGINE_NO_SUCH_ROOM if room does not exist
    }//end of if statement
    if (player_get_position(eng->player, &px, &py) != OK) {
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unable to retrieve player position
    }//end of if statement 

    height = room_get_height(r); //call room_get_height to retrieve room height
    width = room_get_width(r); //call room_get_width to retrieve room width
    if (height <= 0 || width <= 0) {
        return INTERNAL_ERROR; //return INTERNAL_ERROR if room dimensions are invalid
    }//end of if statement
    buffer_s = (size_t)height * (size_t)width; //set the buffer size equal to the height * width with size_t cast

    buffer = malloc(sizeof(char) * buffer_s); //allocate memory for buffer
    if (buffer == NULL) {
        return NO_MEMORY; //return NO_MEMORY error if allocation fails
    }//end of if statement

    if (room_render(r, &eng->charset, buffer, width, height) != OK) { //call room_render and verify success
        free(buffer); //free buffer if unsuccessful 
        return INTERNAL_ERROR; //return INTERNAL_ERROR
    }//end of if statement

    for (int i = 0; i < r->pushable_count; i++) {
        push_x = r->pushables[i].x; //set x coordinate of current pushable
        push_y = r->pushables[i].y; //set y coordinate of current pushable
        if (push_x >= 0 && push_y >= 0 && push_x < width && push_y < height) { //check if coordintes are greater than zero and less than width and height
            buffer[push_y * width + push_x] = eng->charset.pushable; //set the buffer at the calculated index to the engine charset for current pushable
        }//end of if statement
    }//end of for loop

    result = malloc(sizeof(char) * (buffer_s + height + 1)); //allocation memory for result
    if (result == NULL){
        free(buffer); //free buffer if allocation fails 
        return NO_MEMORY; //return NO_MEMORY error
    }//end of if statement

    temp = result; //set temp pointer to result

    //converts 1D character buffer (no linefeeds) into multi-line string (with linefeeds)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            temp[j] = buffer[i * width + j]; //set each temp element to the buffer at the x and y position
        }//end of inner for loop
        temp += width; //add temp to width and set it to that result
        *temp++ = '\n'; //store newline at current index then increment
    }//end of outer for loop
    *temp = '\0'; //set temp pointer to null character

    free(buffer); //free buffer
    *str_out = result; //set str_out pointer to the current result
    return OK;
}//End of game_engine_render_room

Status game_engine_get_room_ids(const GameEngine *eng, int **ids_out, int *count_out){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine, ids out, or count out are NULL
    }//end of if statement
    if (ids_out == NULL || count_out == NULL) {
        return NULL_POINTER; //return NULL_POINTER if ids out or count out are NULL
    }//end of if statement
    //initialize variables
    const void * const *payloads = NULL;
    int count = 0;
    int *ids = NULL;

    if (graph_get_all_payloads(eng->graph, &payloads, &count) != GRAPH_STATUS_OK) { //call graph_get_all_payloads and verify success
        return INTERNAL_ERROR; //return INTERNAL_ERROR if unsuccessful
    }//end of if statement

    ids = calloc(count, sizeof(int)); //allocate memory for ids with calloc
    if (ids == NULL){
        return NO_MEMORY; //return NO_MEMORY if allocation fails
    }//end of if statement

    for (int i = 0; i < count; i++){
        Room *r = (Room *)payloads[i]; //set each room to payload at index i with (Room *) cast
        ids[i] = r->id; //set current id to room id
    }//end of for loop

    *ids_out = ids; //set ids_out to current ids
    *count_out = count; //set count_out to current count
    
    return OK;
}//End of game_engine_get_room_ids

void game_engine_free_string(void *ptr){
    if(ptr == NULL){
        return; //return if pointer is NULL
    }//end of if statement
    free(ptr); //free the given pointer
}//End of game_engine_free_string

Status game_engine_access_portal(GameEngine *eng){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if engine ie NULL
    }//end of if statement
    //initialize variables
    int id = player_get_room(eng->player); //get id of player's current room
    int dest_of_portal = -1;
    int cx = 0;
    int cy = 0;
    Room *r = NULL;
    Room key = {0};

    if (player_get_position(eng->player, &cx, &cy) != OK){ //call player_get_position to find the coordinates of the player
        return INTERNAL_ERROR; //return internal error if unable to retrieve player's position
    }//end of if statement
    key.id = id;
    r = (Room*)graph_get_payload(eng->graph, &key); //set room to the id of the room key  
    if (r == NULL) {
        return INTERNAL_ERROR; //return  INTERNAL_ERROR if room is NULL
    }//end of if statement

    dest_of_portal = room_get_portal_destination(r, cx, cy); //call room_get_portal_destination 
    if (dest_of_portal != -1) { //if exists, find the destination room in the graph using ID
        return check_for_portals(eng, dest_of_portal); //call check_for_portals to check if player can go through the portal
    }//end of if statement
    return ROOM_IMPASSABLE;
}//End of game_engine_access_portal

Status game_engine_get_treasure_total(const GameEngine *eng, int *total_out){
    if (eng == NULL) {
        return INVALID_ARGUMENT; //return INVALID ARGUMENT if eng is NULL
    }//end of if statement
    if (total_out == NULL) {
        return INVALID_ARGUMENT; //return INVALID ARGUMENT if total pointer is NULL
    }//end of if statement

    //initialize variables
    int count = 0;
    int total = 0;
    int *ids = NULL;
    Room key = {0};
    Room *r = NULL;
    
    if(game_engine_get_room_ids(eng, &ids, &count) != OK) { //call game_engine_get_room_ids to retrieve all room ids
        return INTERNAL_ERROR; //return INTERNAL ERROR if unable to retrieve room ids
    }//end of if statement
    for (int i = 0; i < count; i++) {
        key.id = ids[i]; 
        r = (Room*)graph_get_payload(eng->graph, &key); //set room to the id of the room key  
        if (r != NULL) {//check if room is not NULL
            total += r->treasure_count; //set the total treasures to the room treasure count and increment
        }//end of if statement
    }//end of for loop
    free(ids); //free ids from memory
    *total_out = total; //set total treasures to pointer
    return OK; //return OK to verify success
}//End of game_engine_get_treasure_total

Status game_engine_get_adjacency_matrix(const GameEngine *eng, int **matrix_out) {
    if (eng == NULL) {
        return INVALID_ARGUMENT;
    }//end of if statement
    if (matrix_out == NULL) {
        return INVALID_ARGUMENT;
    }//end of if statement
    //initialize variables
    Room key = {0};
    Room *r = NULL;
    int count = eng->room_count;
    int *ids = NULL;
    int count_ids = 0;
    int *matrix = calloc((size_t)count * (size_t)count, sizeof(int)); //allocate memroy for matrix using calloc
    if (matrix == NULL) {
        return NO_MEMORY; //return NO_MEMORY if memory allocation fails
    }//end of if statement

    if (game_engine_get_room_ids(eng, &ids, &count_ids) != OK) { //call game_engine_get_room_ids to retrieve room ids 
        free(matrix); //free matrix if unable to get room ids
        return INTERNAL_ERROR; //return INTERNAL ERROR to indicate unsuccesfful in retrieving ids
    }//end of if statement

    if (count != count_ids) { //check if count is not equal to count ids
        free(ids);  //free ids if not equal
        free(matrix); //also free matrix
        return INTERNAL_ERROR; //return INTERNAL_ERROR to indicte failure
    }//end of if statement

    for (int i = 0; i < count; i++) {
        key.id = ids[i];
        r = (Room*)graph_get_payload(eng->graph, &key); //set room to the id of the room key  
        if (r == NULL) { //check if room is NULL
            free(ids); //free ids if not equal
            free(matrix); //also free matrix
            return INTERNAL_ERROR; //return INTERNAL_ERROR to indicate failure
        }//end of if statement
        if (r->portals != NULL) { //make sure room portals are not NULL
            for (int j = 0; j < count; j++) {
                for (int k = 0; k < r->portal_count; k++) {
                    if (r->portals[k].target_room_id == ids[j]) { //check if room leads to the target room by comparing ids
                        matrix[i * count + j] = 1; //if so then callculate index and set that index to one
                        break; //break out of for loop
                    }//end of inner if statenent
                }//end of k for loop
            }//end of j for loop
        }//end of if statement
    }//end of main for loop
    free(ids); //free ids from memory
    *matrix_out = matrix; //set the matrix to its pointer
    return OK; //return OK to indicate success
}//End of game_engine_get_adjacency_matrix