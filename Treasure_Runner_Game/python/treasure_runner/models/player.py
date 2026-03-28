import ctypes
from ..bindings import lib, Treasure

class Player:
    def __init__(self, ptr):#Constructor
        self._ptr=ptr #stores the pointer
    #End of constructor
    def get_room(self) -> int: #receive the room the player is currently in
        return lib.player_get_room(self._ptr) #call player_get_room function with ptr
    #End of get_room
    def get_position(self) -> tuple[int, int]: #receibe the player's coordinates
        x=ctypes.c_int() #store x coordinate in this variable as int
        y=ctypes.c_int() #store y coordinate in this variable as int
        lib.player_get_position(self._ptr,ctypes.byref(x),ctypes.byref(y)) #call player_get_position C function with x and y variables
        return x.value,y.value #return the x and y coordinates
    #End of get_position
    def get_collected_count(self) -> int: #receive treasure collected count
        return lib.player_get_collected_count(self._ptr) #return integer value of collected count after calling player_get_collected_count with pointer
    #End of get_collected_count
    def has_collected_treasure(self, treasure_id: int) -> bool: #receive boolean of player having specific treasure
        return lib.player_has_collected_treasure(self._ptr,treasure_id) #Call player_has_collected_treasure and return boolean value
    #End of has_collected_treasure
    def get_collected_treasures(self) -> list[dict]: #recieve treasure collected as a list of dicts
        outcome=[] #initialize outcome list for storing treasures
        count=ctypes.c_int() #initialize count as integer to store num of treasures
        t_ptr=lib.player_get_collected_treasures(self._ptr, ctypes.byref(count))#set treasure pointer to the array of treasures from player_get_collected_treasures
        if not t_ptr or count.value==0: #If pointer is NULL or there are zero treasures
            return [] #return empty list
        for i in range(count.value): #iterate through each treasure
            treasure = t_ptr[i].contents #set the current treasure to store its contents
            if treasure.name: #if the treasures name exists
                name= treasure.name.decode('utf-8') #decode the name of treasure
            else:
                name= '' #set name to empty string if otherwise
            t_dict = {#hold each specific keys into python dict
                'id':treasure.id, #set treasure id
                'name':name, #set treasure name
                'starting_room_id':treasure.starting_room_id, #set initial room id
                'initial_x':treasure.initial_x, #set the initial x coordinate
                'initial_y':treasure.initial_y, #set the initial y coordinate
                'x':treasure.x, #set the current x coordinate
                'y':treasure.y, #set the current y coordinate
                'collected':bool(treasure.collected) #set the collected value 
            }#end of dict
            outcome.append(t_dict) #add all converted keys of treasure to resulting list
        return outcome #return the outcome of the list
    #End of get_collected_treasures
