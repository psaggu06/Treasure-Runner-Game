import ctypes
from ..bindings import lib, Status, Direction, Treasure
from .player import Player
from .exceptions import status_to_exception, status_to_status_exception

class GameEngine:
    def __init__ (self, config_path: str): #Constructor
        eng_ptr=ctypes.c_void_p() #initialize a pointer to hold the engine
        status=lib.game_engine_create(config_path.encode('utf-8'), ctypes.byref(eng_ptr)) #call game_engine_create() C Function with encoded string and pointer
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #raise status to corresponding exception if not OK
        if eng_ptr is not None: #verify that pointer is not None
            self._eng= eng_ptr.value #set self to its address
        if self._eng is None: #verify if self is None
            raise RuntimeError("Game Engine Creation Failure") #raise run time error if NULL to indicate game engine failed to create
        player_ptr = lib.game_engine_get_player(self._eng) #call the C function game_engine_get_player and store the value in a new pointer for the player
        if player_ptr is None: #verify if player is NULL
            lib.game_engine_destroy(self._eng) #destroy the game engine with C function game_engine_destroy
            raise RuntimeError("Game Engine Player Retrieval Failure") #raise runtime error if player is NULL to indicate failure of retrieving player
        self._player = Player(player_ptr) #set player pointer to the C player pointer
    #End of Constructor
    @property
    def player(self) -> Player: #property function
        return self._player #return the self._player
    #End of player
    def destroy(self) -> None: #destory engine function
        if self._eng is not None: #check if engine is not NULL
            lib.game_engine_destroy(self._eng) #call game_engine_destroy C function
            self._eng = None #set engine to NULL
    #End of destroy
    def move_player(self, direction: Direction) -> None: #move the player
        status=lib.game_engine_move_player(self._eng, direction) #set status to the players move after calling game_engine_move_player
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #if status is not OK raise status to exception
    #End of move_player
    def render_current_room(self) -> str: #render the current room
        c_char_ptr=ctypes.c_char_p() #initialize string pointer
        status= lib.game_engine_render_current_room(self._eng, ctypes.byref(c_char_ptr)) #call the game_engine_render_current_room function with pointer
        if status!=Status.OK: #verify status
            if c_char_ptr: # check if string exist
                lib.game_engine_free_string(c_char_ptr) #free the string with game_engine_free_string
            raise status_to_exception(status) #raise status_to_exception if failure occured
        if c_char_ptr is not None: #verify string is not null
            outcome=c_char_ptr.value.decode('utf-8') #decode string and set that to the outcome
        lib.game_engine_free_string(c_char_ptr) #call game_engine_free_string to free the string
        return outcome #return the outcome of the function
    #End of render_current_room
    def get_room_count(self) -> int: #retrieve the room count
        count=ctypes.c_int() #initialize count as integer
        status=lib.game_engine_get_room_count(self._eng,ctypes.byref(count)) #call game_engine_get_room_count function with int
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #raise exception if status is not OK
        return count.value #return the number of rooms
    #End of get_room_count
    def get_room_dimensions(self) -> tuple[int, int]: #retrieve the dimensions of the room
        height= ctypes.c_int() #initialize height as integer
        width= ctypes.c_int() #initialize width as integer
        status=lib.game_engine_get_room_dimensions(self._eng, ctypes.byref(width), ctypes.byref(height)) #call game_engine_get_room_dimensions with width and height
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #raise exception if status is not OK
        return width.value,height.value #return width and height values
    #End of get_room_dimensions
    def get_room_ids(self) -> list[int]: #retrieve the ids of the rooms
        count= ctypes.c_int() #initialize count as int
        ids_ptr=ctypes.POINTER(ctypes.c_int)() #initialize ids as int pointer
        status=lib.game_engine_get_room_ids(self._eng, ctypes.byref(ids_ptr), ctypes.byref(count)) #call game_engine_get_rooms_ids with count and pointer
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #return exception if status is not OK
        outcome = [] #initialize outcome as empty list/array
        if ids_ptr and count.value > 0: #check if ids pointer and count is positive
            for i in range (count.value): #iterate through list
                outcome.append(ids_ptr[i]) #append each C integer to python array
        return outcome #return resulting list
    #End of get_room_ids
    def access_portal(self) -> None: #allow player to enter portal if they are standing on one
        status=lib.game_engine_access_portal(self._eng) #call game_engine_access_portal
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #raise exception if status is not OK
    #End of access_portal
    def get_treasure_total(self) -> int: #retrieve the total amount of treasures in the world
        total_treasures=ctypes.c_int() #initialize total treasures as an integer
        status=lib.game_engine_get_treasure_total(self._eng, ctypes.byref(total_treasures)) #call game_engine_get_treasures_total with treasure total pointer
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #raise exception if status is not OK
        return total_treasures.value #return the total amount of treasures
    #End of get_treasure_total
    def get_matrix(self) -> list[list[int]]:
        matrix_ptr=ctypes.POINTER(ctypes.c_int)() #initialize matrix pointer
        status=lib.game_engine_get_adjacency_matrix(self._eng, ctypes.byref(matrix_ptr)) #call game_engine_get_adjacency_matrix with matrix pointer
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #raise exception if status is not ok
        count=self.get_room_count() #initialize count and set it to the room count
        matrix=[] #initialize matrix as list
        if matrix_ptr and count>0: #check if matrix pointer exists and count is positive
            for i in range(count): #iterate through each row
                row=[] #initialize rows
                for j in range(count): #iterate through each column
                    row.append(matrix_ptr[i*count+j]) #convert coordinates into a single index
                matrix.append(row) #add the entire row to the matrix after completing conversion
        return matrix #return the completed matrix
    #End of get_matrix
    def reset(self) -> None: #reset the game engine
        status=lib.game_engine_reset(self._eng) #call the game_engine_reset C function to reset the game engine
        if status!=Status.OK: #verify status
            raise status_to_exception(status) #rasie exception if status is not OK
    #End of reset
