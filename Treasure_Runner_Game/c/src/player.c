#include "player.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

Status player_create(int initial_room_id, int initial_x, int initial_y, Player **player_out){
    if (player_out == NULL) {
        return INVALID_ARGUMENT; //return invalid argument error if output pointer is NULL
    }//end of if statement

    Player *p = malloc(sizeof(Player)); //initialize new player and allocate memory for new player
    if (p == NULL) {
        return NO_MEMORY; //return no memory error if allocation fails
    }//end of if statement

    p->room_id = initial_room_id; //set the new player room id and position to initial
    p->x = initial_x; //set the player's x position to initial x position
    p->y = initial_y; //set the player's y position to initial y position
    p->collected_treasures = NULL; //initialize collected treasures array to NULL
    p->collected_count = 0; //initialize collected count to 0
    *player_out = p; //set pointer to store new player

    return OK;
}//End of player_create

void player_destroy(Player *p){
    if (p == NULL) {
        return; //return if player is already NULL
    }//end of if statement
    if (p->collected_treasures != NULL) {
        free(p->collected_treasures); //free the collected treasures array if it is not NULL
    }//end of if statement
    free(p); //free the memory allocated for the player
}//End of player_destroy

int player_get_room(const Player *p){
    if (p == NULL) {
        return -1; //return -1 if player is NULL
    }//end of if statement
    return p->room_id; //return the current room id of the player
}//End of player_get_room

Status player_get_position(const Player *p, int *x_out, int *y_out){
    if (p == NULL || x_out == NULL || y_out == NULL) {
        return INVALID_ARGUMENT; //return invalid argument error if player or output pointers are NULL
    }//end of if statement
    *x_out = p->x; //copy the x position to player's x position
    *y_out = p->y; //copy the y position to player's y position
    return OK; //return OK to indicate success
}//End of player_get_position

Status player_set_position(Player *p, int x, int y){
    if (p == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if player is already NULL
    }//end of if statement
    p->x = x; //copy the player's x position to new x position
    p->y = y; //copy the player's y position to new y position
    return OK; //return OK to indicate success
}//End of player_set_position

Status player_move_to_room(Player *p, int new_room_id){
    if (p == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if player is already NULL
    }//end of if statement
    p->room_id = new_room_id; //copy the player's room id to the new room id
    return OK; //return OK on success
}//End of player_move_to_room

Status player_reset_to_start(Player *p,int starting_room_id, int start_x, int start_y){
    if (p == NULL) {
        return INVALID_ARGUMENT; //return INVALID_ARGUMENT if p is alrady NULL
    }//end of if statement
    p->room_id=starting_room_id; //set player room id to the starting room id
    p->x = start_x; //set player x position to starting x position
    p->y = start_y; //set player y position to starting y position
    p->collected_count = 0; //reset collected count to 0
    free(p->collected_treasures); //free the collected treasures array
    p->collected_treasures = NULL; //set collected treasures array to NULL
    return OK; //return OK to indicate success
}//End of player_reset_to_start

Status player_try_collect(Player *p, Treasure *treasure){
    if (p == NULL || treasure == NULL) {
        return NULL_POINTER; //return invalid argument error if player or treasure is NULL
    }//end of if statement
    if (treasure->collected) {
        return INVALID_ARGUMENT; //return invalid argument error if treasure has already been collected
    }//end of if statement

    if (player_has_collected_treasure(p, treasure->id)) {
        return OK; //return OK if player has already collected the treasure
    }//end of if statement

    Treasure **new_collected = realloc(p->collected_treasures,sizeof(Treasure*)*(p->collected_count+1)); //allocate memory for new collected treasures array with space for one more treasure
    if (new_collected == NULL) {
        return NO_MEMORY; //return NO_MEMORY error if allocation fails
    }//end of if statement

    p->collected_treasures = new_collected; //set player's collected treasures to the new array
    p->collected_treasures[p->collected_count] = treasure; //add the new treasure to the end of the collected treasures array
    p->collected_count++; //increment the count of collected treasures
    treasure->collected = true; //set the treasure's collected status to true
    return OK; //return OK to indicate success
}//End of player_try_collect

bool player_has_collected_treasure(const Player *p, int treasure_id){
    if(p==NULL){
        return false; //return falseT if player is NULL
    }//end of if statement
    if(treasure_id<0){
        return false; //return false if negative id value is passed
    }//end of if statement
    
    for(int i=0;i<p->collected_count;i++){
        if(p->collected_treasures[i]->id==treasure_id){
            return true; //return true if treasure with given id is found in player's collected treasures
        }//end of if statement
    }//end of for loop
    return false; //return false if treasure with given id is not found in player's collected treasures
}//End of player_has_collected_treasure

int player_get_collected_count(const Player *p){
    if(p==NULL){
        return 0; //return 0 if NULL
    }//end of if statement
    return p->collected_count; //return the count of collected treasures for the player
}//End of player_get_collected_count

const Treasure * const *player_get_collected_treasures(const Player *p, int *count_out){
    if(p==NULL||count_out==NULL){
        return NULL; //return NULL if player or count output pointer is NULL
    }//end of if statement
    *count_out = p->collected_count; //copy the count of collected treasures to output parameter
    return (const Treasure * const *)p->collected_treasures; //return the array of collected treasures
}//End of player_get_collected_treasures