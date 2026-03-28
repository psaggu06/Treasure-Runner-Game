#include "room.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

static void portals_and_treasures_rendering(const Room *r, const Charset *cs, char *buffer) {
    int tx = 0;
    int ty = 0;
    int px = 0;
    int py = 0;
    if (r->treasures != NULL) {
        for (int i = 0; i < r->treasure_count; i++) {
            tx = r->treasures[i].x; //set each treasure x position
            ty = r->treasures[i].y; //set each treasure y position
            if (tx >= 0 && ty >= 0 && tx < r->width && ty < r->height) {
                if (!r->treasures[i].collected) {
                    buffer[ty * r->width + tx] = cs->treasure; //set buffer at treasure position to charset treasure
                }//end of inner if statement
            }//end of if statement
        }//end of for loop
    }//end of if statement
    if (r->portals != NULL) {
        for (int i = 0; i < r->portal_count; i++) {
            px = r->portals[i].x; //set each portal x position
            py = r->portals[i].y; //set each portal y position
            if (px >= 0 && py >= 0 && px < r->width && py < r->height) {
                buffer[py * r->width + px] = cs->portal; //set buffer at portal position to charset portal
            }//end of if statement
        }//end of for loop
    }//end of if statement
}//End of render_portals_and_treasures

Room *room_create(int id, const char *name, int width, int height){
    Room *r = malloc(sizeof(Room)); //allocate memory for room
    if(r == NULL) {
        return NULL; //return NULL if room is null
    }//end of if statement
    //set all room attributes
    r->id = id;
    if (width < 1) {
        width = 1; //set width to 1 if less than 1
    }//end of if statement
    if (height < 1) {
        height = 1; //set height to 1 if less than 1
    }//end of if statement
    //set each room attribute  
    r->width = width;
    r->height = height;
    r->floor_grid = NULL;
    r->portals = NULL;
    r->portal_count = 0;
    r->treasures = NULL;
    r->treasure_count = 0;
    r->pushables = NULL;
    r->pushable_count = 0;
    if (name == NULL) {
        r->name = NULL; //set room name to null if name is NULL
    } else {
        r->name = malloc((strlen(name) + 1) * sizeof(char)); //allocate memory for the room name
        if (r->name == NULL) {
            free(r); //free room if room name is NULL
            return NULL; //return NULL
        }//end of inner if-else statement
        strcpy(r->name, name); //copy name to room name
    }//end of inner if-else
    return r; 
}//End of room_create

int room_get_width(const Room *r){
    if (r == NULL) {
        return 0; //return 0 if room is NULL
    }//end of if statement
    return r->width; //return the width of the room
}//End of room_get_width

int room_get_height(const Room *r){
    if (r == NULL) {
        return 0; //return 0 if room is NULL
    }//end of if statement
    return r->height; //return the height of the room
}//End of room_get_height

Status room_set_floor_grid(Room *r, bool *floor_grid){
    if (r == NULL) {
        return INVALID_ARGUMENT; //return invalid argument if r is NULL
    }//end of if statement

    if (r->floor_grid != NULL) {
        free(r->floor_grid); //free the room floor grid if previously initialized
    }//end of if statement

    r->floor_grid = floor_grid; //set the room floor grid to the new floor grid 
    return OK;
}//End of room_set_floor_grid

Status room_set_portals(Room *r, Portal *portals, int portal_count){
    if (r == NULL) {
        return INVALID_ARGUMENT; //return invalid argument if r is NULL
    }//end of if statement

    if ((portal_count > 0 && portals == NULL) || portal_count < 0) {
        return INVALID_ARGUMENT; //return invalid argument if parameters are invalid for portal or portal count
    }//end of if statement

    if (r->portals != NULL) {
        for (int i = 0; i < r->portal_count; i++) { 
            free(r->portals[i].name); //free each portal name if previously initialized
        }//end of for loop
        free(r->portals); //free room portals
    }//end of if statement

    r->portals = portals; //set room portal to new portals
    r->portal_count = portal_count; //set room portal count to new portal count
    return OK;
}//End of room_set_portals

Status room_set_treasures(Room *r, Treasure *treasures, int treasure_count){
    if (r == NULL) {
        return INVALID_ARGUMENT; //return invalid argument if r is NULL
    }//end of if statement

    if ((treasure_count > 0 && treasures == NULL) || treasure_count < 0) {
        return INVALID_ARGUMENT; //return invalid argument if treasure count and treasures parameters are not valid
    }//end of if statement

    if (r->treasures != NULL) {
        for (int i = 0; i < r->treasure_count; i++) {
            free(r->treasures[i].name); //free each treasure name if previously initialized
        }//end of for loop
        free(r->treasures); //free room treasures
    }//end of if statement

    r->treasures = treasures; //set room treasures to new treasures
    r->treasure_count = treasure_count; //set room treasure count to new treasure count
    return OK;
}//End of room_set_treasures

Status room_place_treasure(Room *r, const Treasure *treasure){
    if (r == NULL || treasure == NULL) {
        return INVALID_ARGUMENT; //return invalid argument if r or treasure is NULL
    }//end of if statement
    
    char *name = NULL; //initialize name for treasure and set to NULL
    if (treasure->name != NULL) {
        name = malloc((strlen(treasure->name) + 1) * sizeof(char)); //allocate memory for treasure name
        if (name == NULL) {
            return NO_MEMORY; //return NO_MEMORY if allocation fails
        }//end of inner if statement
        strcpy(name, treasure->name); //copy name into treasure name
    }//end of if statement

    Treasure *newt = realloc(r->treasures, sizeof(Treasure) * (r->treasure_count + 1)); //reallocate memory for new treasure 
    if (newt == NULL) {
        free(name); //free name if reallocation fails
        return NO_MEMORY; //return no memory if null
    }//end of if statement

    //set new treasure attributes
    r->treasures = newt;
    r->treasures[r->treasure_count] = *treasure;
    r->treasures[r->treasure_count].name = name;
    r->treasures[r->treasure_count].collected = false; 
    r->treasure_count++;

    return OK;
}//End of room_place_treasure

int room_get_treasure_at(const Room *r, int x, int y){
    if (r == NULL || r->treasures == NULL || x < 0 || y < 0 || x >= r->width || y >= r->height) {
        return -1; //return -1 if room or room treasure is null, or if x or y are out of bounds
    }//end of if statement

    for (int i = 0; i < r->treasure_count; i++) {
        if (r->treasures[i].x == x && r->treasures[i].y == y && !r->treasures[i].collected) {
            return r->treasures[i].id; //return current room treasure id if equal to given dimensions and treasure is not collected
        }//end of if statement
    }//end of for loop
    return -1; //return -1 if unable to retrieve treasure id
}//End of room_get_treasure_at

int room_get_portal_destination(const Room *r, int x, int y){
    if (r == NULL || r->portals == NULL || x < 0 || y < 0 || x >= r->width || y >= r->height) {
        return -1; //return -1 if room or room portal is null, or if x or y are out of bounds
    }//end of if statement

    for(int i = 0; i < r->portal_count; i++) {
        if (r->portals[i].x == x && r->portals[i].y == y) {
            return r->portals[i].target_room_id; //return current portal target room id if given dimensions match 
        }//end of if statement
    }//end of for loop
    return -1; //return -1 if unable to retrieve portal target room id
}//End of room_get_portal_destination

bool room_is_walkable(const Room *r, int x, int y){
    if (r == NULL) {
        return false; //return false if room is null
    }//end of if statement

    if (x < 0 || y < 0 || x >= r->width || y >= r->height) {
        return false; //return false if x or y are out of bounds
    }//end of if statement

    if (room_has_pushable_at(r, x, y, NULL)) {
        return false; //return false if there is a pushable at the given coordinates
    }//end of if statement
    
    if (r->floor_grid != NULL) {
        return r->floor_grid[y * r->width + x]; //return the room floor grid at calculated index
    }//end of if statement

    if (x == 0 || y == 0 || x == r->width-1 || y == r->height-1) { 
        return false; //return false if wall on perimeter
    }//end of if statement

    return true;
}//End of room_is_walkable


RoomTileType room_classify_tile(const Room *r,int x, int y, int *out_id){
    //initialize variables
    int portal_destination = -1; 
    int treasure_id = -1;
    int psh_idx = -1;

    if (r == NULL || x < 0 || x >= r->width || y < 0 || y >= r->height) {
        return ROOM_TILE_INVALID; //return room_title_invalid if room is null or x or y are out of bounds
    }//end of if statement

    if (out_id != NULL) {
        *out_id = -1; //set out_id to -1 if not null
    }//end of if statement

    if (room_has_pushable_at(r, x, y, &psh_idx)) {
        if (out_id != NULL) {
            *out_id = psh_idx; //set out_id to pushable index
        }//end of if statement
        return ROOM_TILE_PUSHABLE; //return room_tile_pushable to indicate the tile is a pushable
    }//end of if statement

    treasure_id = room_get_treasure_at(r, x, y); //call room_get_treasure_at and set treasure id to that value
    if (treasure_id != -1) {
        if (out_id != NULL) { 
            *out_id = treasure_id; //set out_id to treasure_id
        }//end of innner if statement 
        return ROOM_TILE_TREASURE; //return room_tile_treasure to indicate the tile is a treasure
    }//end of if statement

    if (!room_is_walkable(r, x, y)) {
        return ROOM_TILE_WALL; //return room_tile_wall if room is not walkable
    }//end of if statement

    portal_destination = room_get_portal_destination(r, x, y); //call room_get_portal_destination
    if (portal_destination != -1) {
        if (out_id != NULL) { 
            *out_id = portal_destination; //set out_id to portal destination
        }//end of if statement 
        return ROOM_TILE_PORTAL; //return room_tile_portal to indicate the tile is a portal
    }//end of if statement

    return ROOM_TILE_FLOOR; //return room_tile_floor to indicate the tile is a floor
}//End of room_classify_tile

Status room_render(const Room *r, const Charset *charset, char *buffer, int buffer_width, int buffer_height){
    if (r == NULL || charset == NULL || buffer == NULL || buffer_width != r->width || buffer_height != r->height) {
        return INVALID_ARGUMENT; //return invalid arguement if room, charset, or buffer are NULL, or if either the buffer height or width are not equal to the room height or width
    }//end of if statement

    if (r->floor_grid != NULL) {
        for (int i = 0; i < r->height; i++) {
            for (int j = 0; j < r->width; j++) {
                if (r->floor_grid[i * r->width + j]) { 
                    buffer[i * r->width + j] = charset->floor; //set buffer at current index to the charset floor
                } else {
                    buffer[i * r->width + j]= charset->wall; //set buffer at current index to the charset wall
                }//end of if-else statement
            }//end of inner for loop
        }//end of outer for loop
    } else {
        for (int i = 0; i < r->height; i++) {
            for (int j = 0; j < r->width; j++) {
                if (j == 0 || i == 0 || j == r->width - 1 || i == r->height - 1) {
                    buffer[i * r->width + j] = charset->wall;  //set buffer at current index to the charset wall
                } else { 
                    buffer[i * r->width + j] = charset->floor; //set buffer at current index to the charset floor
                }//end of inner if-else statement 
            }//end of inner for loop
        }//end of outer for loop
    }//end of if-else statement
    portals_and_treasures_rendering(r, charset, buffer); //call render_portals_and_treasures 
    return OK; 
}//End of room_render

Status room_get_start_position(const Room *r, int *x_out, int *y_out){
    if (r == NULL || x_out == NULL || y_out == NULL) {
        return INVALID_ARGUMENT; //return invalid_arguement if either room, x_out, or y_out is NULL
    }//end of if statement
    
    for (int i = 0; i < r->portal_count; i++) {
        if (room_is_walkable(r, r->portals[i].x, r->portals[i].y)) {
            *x_out = r->portals[i].x; //set x_out to portal x position 
            *y_out = r->portals[i].y; //set y_out to portal y position
            return OK;
        }//end of if statement
    }//end of for loop

    for (int i = 0; i < r->height; i++) {
        for (int j = 0; j < r->width; j++) {
            if (room_is_walkable(r, j, i)) {
                *x_out = j; //set x_out to x position if room is walkable
                *y_out = i; //set y_out to y position if room is walkable
                return OK;
            }//end of if statement 
        }//end of inner for loop
    }//end of outer for loop
    return ROOM_NOT_FOUND; //return to indicate room could not be found
}//End of room_get_start_position

void room_destroy(Room *r){
    if (r == NULL) {
        return; //exit function if room is already NULL
    }//end of if statement
    
    if (r->portals != NULL) {
        for (int i = 0; i < r->portal_count; i++) {
            if (r->portals[i].name != NULL) {
                free(r->portals[i].name); //free each portal name
            } //end of inner if statement
        }//end of for loop
        free(r->portals); //free the portals themselves
    }//end of outer if statement

    if (r->treasures != NULL) {
        for (int i = 0; i < r->treasure_count; i++) {
            if (r->treasures[i].name != NULL) {
                free(r->treasures[i].name); //free each treasure name
            } //end of inner if statement
        }//end of for loop
        free(r->treasures); //free the treasures themselves
    }//end of outer if statement 

    if (r->pushables != NULL) {
        for (int i = 0; i < r->pushable_count; i++) {
            if (r->pushables[i].name != NULL) {
                free(r->pushables[i].name); //free each pushable name
            }//end of inner if statement
        }//end of for loop
        free(r->pushables); //free the pushables themselves
    }//end of outer if statement

    if (r->name != NULL) {
        free(r->name); //free room name
    }//end of if statement
        
    if (r->floor_grid) {
        free(r->floor_grid); //free room floor grid
    }//end of if statement

    free(r); //free the room itself
}//End of room_destroy

int room_get_id(const Room *r){
    if (r == NULL) {
        return -1; //return -1 if room is null
    }//end of if statement
    return r->id; //return the room id
}//End of room_get_id

Status room_pick_up_treasure(Room *r, int treasure_id, Treasure **treasure_out){
    if (r == NULL || treasure_out == NULL || treasure_id < 0) {
        return INVALID_ARGUMENT; //return invalid argument if room or treasure output pointer is null, or if treasure id is negative
    }//end of if statement

    for (int i = 0; i < r->treasure_count; i++) {
        if (r->treasures[i].id == treasure_id) {
            if (r->treasures[i].collected) {
                return INVALID_ARGUMENT; //return invalid argument if treasure has already been collected
            }//end of inner if statement
            r->treasures[i].collected = true; //set the treasure collected status to true
            *treasure_out = &r->treasures[i]; //set the treasure output pointer to the current treasure
            return OK; //return OK to indicate success
        }//end of if statement
    }//end of for loop
    return ROOM_NOT_FOUND; //return room not found if no matching treasure id is found in the room
}//End of room_pick_up_treasure

void destroy_treasure(Treasure *t){
    if (t == NULL) {
        return; //exit function if treasure is already NULL
    }//end of if statement

    if (t->name != NULL) {
        free(t->name); //free treasure name
    }//end of if statement

    free(t); //free the treasure itself
}//End of destroy_treasure

bool room_has_pushable_at(const Room *r, int x, int y, int *pushable_idx_out){
    if(r == NULL || x < 0 || y < 0 || x >= r->width || y >= r->height) {
        return false; //return false if room is null or if x or y are out of bounds
    }//end of if statement
    for (int i = 0; i < r->pushable_count; i++) {
        if (r->pushables[i].x == x && r->pushables[i].y == y) {
            if (pushable_idx_out != NULL) {
                *pushable_idx_out = i; //set pushable index output to current index if not null
            }//end of if statement
            return true; //return true if pushable is found at given coordinates
        }//end of if statement
    }//end of for loop
    return false; //return false if no pushable is found at given coordinates
}//End of room_has_pushable_at

Status room_try_push(Room *r, int pushable_idx, Direction dir){
    if (r == NULL || pushable_idx < 0 || pushable_idx >= r->pushable_count || dir < 0 || dir > 3) {
        return INVALID_ARGUMENT; //return invalid argument if room is null, or if pushable index is out of bounds, or if direction is invalid
    }//end of if statement
    int new_x = r->pushables[pushable_idx].x; //initialize new x position to current pushable x position
    int new_y = r->pushables[pushable_idx].y; //initialize new y position to current pushable y position
    if (dir == DIR_NORTH){
        new_y--; //up
    } else if (dir == DIR_WEST){
        new_x--; //left
    } else if (dir == DIR_SOUTH){
        new_y++; //down
    } else if (dir == DIR_EAST){
        new_x++; //right
    } else {
        return INVALID_ARGUMENT; //return invalid argument if direction is invalid
    }//end of if-else statements
    if (room_is_walkable(r, new_x, new_y)) { 
        r->pushables[pushable_idx].x = new_x; //update pushable x position to new x position
        r->pushables[pushable_idx].y = new_y; //update pushable y position to new y position
        return OK; //return ok to indicate successful push
    }//end of if statement
    return ROOM_IMPASSABLE; //return room not found if push is not possible due to un
}//End of room_try_push