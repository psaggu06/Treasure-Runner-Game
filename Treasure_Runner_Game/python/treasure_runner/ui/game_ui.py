import curses
import json
from treasure_runner.models.exceptions import ImpassableError
from treasure_runner.bindings import Direction
#The curses implementation in this assignment is used from CIS*2750 lectures from MVC_Curses (March 12 2026), and More_Curses (March 17 2026)
class GameUI:
    def __init__(self,stdscr,engine,profile,profile_path): #constructor
        self.stdscr=stdscr #set the standard screen
        self.engine=engine #set the game engine for game logic
        self.profile=profile #set the profile of player
        self.profile_path=profile_path #set the profile path for the JSON file
        self.rooms_visited={self.engine.player.get_room()} #set rooms visited to player's current room
        self.total_treasures=self.engine.get_treasure_total() #set the total treasures of the world
        self.message="This is Treasure Runner! Collect Treasures and Find the Portals!" #set self.message to a opening message that displays when a player begins the game
        curses.curs_set(0) #set the curses set to hide blinking cursor in terminal
        self.stdscr.keypad(True) #set the standard screen arrow keys to true
        curses.start_color() #activiate colors
        curses.use_default_colors() #enable the default curses colours
        curses.init_pair(1,curses.COLOR_GREEN,-1) #set green to 1
        curses.init_pair(2,curses.COLOR_YELLOW,-1) #set yellow to 2
        curses.init_pair(3,curses.COLOR_BLUE,-1) #set blue to 3
        curses.init_pair(4,curses.COLOR_RED,-1) #set red to 4
        curses.init_pair(5,curses.COLOR_MAGENTA,-1) #set magenta to 5
        curses.init_pair(6,curses.COLOR_CYAN,-1) #set cyan to 6
    #End of constructor
    def live_stats(self): #used to update player statistics live as they're playing the game
        current_collected_count=self.engine.player.get_collected_count() #call get_collected_count and set it to the current collected count
        if current_collected_count>self.profile['max_treasure_collected']: #check if player has collected more than their max
            self.profile['max_treasure_collected']=current_collected_count #update the player's max treasures collected amount
        current_room=self.engine.player.get_room() #set the current room to the player's current room they're in
        self.rooms_visited.add(current_room) #add current room to the list of rooms visited
        if len(self.rooms_visited)>self.profile['most_rooms_world_completed']: #if the amount of rooms visited is greater than the players most room world completed
            self.profile['most_rooms_world_completed']=len(self.rooms_visited) #update the players most rooms world completed
    #End of live_stats
    def save_profile(self): #save profile of the player
        with open(self.profile_path,'w',encoding='utf-8') as file: #open JSON file in write mode
            json.dump(self.profile,file,indent=4) #dump the updated profile disctionary back into the file
    #End of save_profile
    def splash_screen(self):
        self.stdscr.clear() #clear the standard screen to make room for the splash screen
        self.stdscr.addstr(2,5,"TREASURE RUNNER",curses.A_BOLD|curses.color_pair(2)) #display the name of the game in bold and in yellow
        self.stdscr.addstr(4,5,"Player Statistics Pre-Game:",curses.A_BOLD) #display this message to show player stats in bold
        self.stdscr.addstr(5,8,f"Player: {self.profile['player_name']}") #display player's name
        self.stdscr.addstr(6,8,f"Games Played: {self.profile['games_played']}") #display the amount of games played
        self.stdscr.addstr(7,8,f"Maximum Treasures: {self.profile['max_treasure_collected']}") #display the maximum treasures collected
        self.stdscr.addstr(8,8,f"Biggest World Completed: {self.profile['most_rooms_world_completed']}") #display the biggest world completed by the player
        self.stdscr.addstr(9,8,f"Last Played: {self.profile['timestamp_last_played']}") #display the last time the user played the game
        self.stdscr.addstr(11,5, "Press any key to play!",curses.color_pair(1)) #Tell the user to press any key to start the treasure runner game in green
        self.stdscr.refresh() #refresh the standard screen
        self.stdscr.getch() #wait for final keystroke before closing program
    #End of splash_screen
    def game_over_screen(self):
        self.stdscr.clear() #clear the standard screen to make room for the game over screen
        self.stdscr.addstr(2,5,"GAME OVER...",curses.A_BOLD|curses.color_pair(4)) #display "GAME OVER" in bold and in red
        self.stdscr.addstr(4,5,"Player Statistics Post-Game:",curses.A_BOLD) #display this message to show player stats after game ends in bold
        self.stdscr.addstr(5,8,f"Player: {self.profile['player_name']}") #display the players name
        self.stdscr.addstr(6,8,f"Games Played: {self.profile['games_played']}") #display the amount of games this player has played
        self.stdscr.addstr(7,8,f"Maximum Treasures: {self.profile['max_treasure_collected']}") #display the maximum treasures collected
        self.stdscr.addstr(8,8,f"Biggest World Completed: {self.profile['most_rooms_world_completed']}") #display the biggest world completed by the player
        self.stdscr.addstr(9,8,f"Last Played: {self.profile['timestamp_last_played']}") #display the last time the user played the game
        self.stdscr.addstr(11,5, "Press any key to exit",curses.color_pair(1)) #tell the user to press any key to terminate the game in green
        self.stdscr.refresh() #refresh the standard screen
        self.stdscr.getch() #wait for final keystroke before closing program
    #End of game_over_screen
    def victory_screen(self):
        self.stdscr.clear() #clear the standard screen to make room for the victory screen
        self.stdscr.addstr(2,5,"VICTORY!!!",curses.A_BOLD|curses.color_pair(6)) #display victory message on screen in bold and in cyan
        self.stdscr.addstr(4,5,"You have collected all the treasures. Congratulations!",curses.color_pair(2)) #display seperate message in yellow indicating all treasures have been collected
        self.stdscr.addstr(6,5,"Player Statistics Post-Game:",curses.A_BOLD) #display this message to show player stats after game ends in bold
        self.stdscr.addstr(7,8,f"Player: {self.profile['player_name']}") #display the players name
        self.stdscr.addstr(8,8,f"Games Played: {self.profile['games_played']}") #display the amount of games this player has played
        self.stdscr.addstr(9,8,f"Maximum Treasures: {self.profile['max_treasure_collected']}") #display the player's maximum treasures collected
        self.stdscr.addstr(10,8,f"Biggest World Completed: {self.profile['most_rooms_world_completed']}") #display the biggest world completed by the player
        self.stdscr.addstr(11,8,f"Last Played: {self.profile['timestamp_last_played']}") #display the last time the user played the game
        self.stdscr.addstr(13,5,"Press any key to exit",curses.color_pair(1)) #tell the player to press any game to terminate the game in green
        self.stdscr.refresh() #refresh the standard screen
        self.stdscr.getch() #wait for final keystroke before closing program
    #End of victory_screen
    def run(self): #run the game
        self.splash_screen() #call splash_screen to display the opening splashs screen to the user
        while True: #Run game until user wins or quits
            self.live_stats() #call live_stats to display the player's statistics updating live as they play the game
            self.render_screen() #call render_screen to render the entire display on terminal
            if self.total_treasures>0 and self.engine.player.get_collected_count()==self.total_treasures: #check if world treasures exist and the player's collected amount of treasures are the same as the world treasures
                self.save_profile() #save the player's stats
                self.victory_screen() #display the victory splash screen to tell the user they have beat the world
                break #break out of while loop
            key=self.stdscr.getch() #wait for user to press a key to unpause program
            if key==ord('q'): #check if player's key input was the q key
                self.save_profile() #save the player's stats
                self.game_over_screen() #display the game over splash screen to tell the user that they have exited the game
                break #break out of while loop
            self.input_handling(key) #call input_handling for any other input that has been made besides quitting
    #End of run
    def _render_map(self): #helper function for render_screen, draws every char and assigns each char a colour
        map_render=self.engine.render_current_room() #call C function render_current_room to retrieve the entire current room
        for row_idx,line in enumerate(map_render.split('\n')): #split map string with newline and iterate through each row
            for col_idx,char in enumerate(line): #also iterate through each column
                if char=='@': #check if char is the player symbol
                    self.stdscr.addstr(row_idx+3,col_idx,char,curses.color_pair(1)) #draw and set green colour to player
                elif char=='$': #check if char is the treasure symbol
                    self.stdscr.addstr(row_idx+3,col_idx,char,curses.color_pair(2)) #draw and set yellow colour to treasure
                elif char=='X': #check if char is the portal symbol
                    self.stdscr.addstr(row_idx+3,col_idx,char,curses.color_pair(3)) #draw and set the blue colour to portal
                elif char=='#': #check if char is the wall symbol
                    self.stdscr.addstr(row_idx+3,col_idx,char,curses.color_pair(4)) #draw and set the red colour to wall
                elif char=='O': #check if char is the boulder/pushable symbol
                    self.stdscr.addstr(row_idx+3,col_idx,char,curses.color_pair(5)) #draw and set the magenta colour for pushable
                else: #if char is a floor...
                    self.stdscr.addstr(row_idx+3,col_idx,char) #draw floor with default colour (white)
    #End of _render_map
    def _render_legend(self,legend): #helper function for render_screen(), displays the legend on screen to tell the user what each symbol represents in the gane
        self.stdscr.addstr(3,legend,"Game Elements (Legend):",curses.A_BOLD) #display the title of the legend in bold
        self.stdscr.addstr(5,legend,"@ - Player",curses.color_pair(1)) #print player synbol with appropriate colour
        self.stdscr.addstr(6,legend,"# - Wall",curses.color_pair(4)) #print wall synbol with appropriate colour
        self.stdscr.addstr(7,legend,"$ - Treasure",curses.color_pair(2)) #print treasure synbol with appropriate colour
        self.stdscr.addstr(8,legend,"X - Portal",curses.color_pair(3)) #print portal synbol with appropriate colour
        self.stdscr.addstr(9,legend,"O - Pushable",curses.color_pair(5)) #print pushable synbol with appropriate colour
        self.stdscr.addstr(10,legend,". - Floor") #print floor synbol with appropriate colour
    #End of _render_legend
    def _render_status(self,bottom): #helper function for render_screen(), displays the players live stats while game occurs along witb the game controls
        self.stdscr.addstr(bottom,0,"Controls: WASD/Arrow Keys: Movement | >: Enter Portal | R: Reset World | Q: Quit Game") #display game controls on screen so player knows what inputs are required
        #set the player's statistics in a status bar
        status_bar=f"Player: {self.profile['player_name']} | " f"Treasure: {self.engine.player.get_collected_count()}/{self.total_treasures} | " f"Maximum Treasures: {self.profile['max_treasure_collected']} | " f"Rooms Explored: {len(self.rooms_visited)}"
        self.stdscr.addstr(bottom+2,0,status_bar) #display the status bar below the controls
        game_name="Treasure Runner" #set the name of the game to "Treasure RUnner"
        email="psaggu@uoguelph.ca" #set the email to my student email
        self.stdscr.addstr(bottom+4,0,game_name) #display the game name below everything
        self.stdscr.addstr(bottom+4,45,email) #display the student on the same row beside the game name
    #End of _render_status
    def _render_minimap(self, row, col): #helper function for _render_screen(), display a minimap for the rooms in the world they player has visited and how many rooms exist
        self.stdscr.addstr(row,col,"Minimap: ", curses.A_BOLD) #display Minimap title in bold
        matrix=self.engine.get_matrix() #retrieve the adjacency matrix
        room_ids=self.engine.get_room_ids() #retrieve the room ids
        for i, r_id in enumerate(room_ids): #iterate through all rooms in the world
            portals=[]#initialize portals to find the rooms this current room connects to
            for j, target_id in enumerate(room_ids): #iterate through all room target ids
                if matrix[i][j]==1 or matrix[j][i]==1: #then check the matrix to see if two rooms connect to eachother one way or the other
                    portals.append(str(target_id)) #convert target room id to string and add to list of portals
            if r_id==self.engine.player.get_room(): #check if the current room id is the same as the current room
                self.stdscr.addstr(row+2+(i*2),col+2,f"Room: {r_id}",curses.color_pair(3)|curses.A_REVERSE) #set that room id to colour blue in reverse to tell the user thats the room they're currently in
                self.stdscr.addstr(row+2+(i*2),col+10,f"(Portals Available: {', '.join(portals)})",curses.color_pair(3)) #print the portals beside the room number to show what the portals goes to
            elif r_id in self.rooms_visited: #check if room id has already been visited
                self.stdscr.addstr(row+2+(i*2),col+2,f"Room: {r_id}",curses.color_pair(1)|curses.A_BOLD) #set that room id to colour green in bold to tell the user they have visited that room
                self.stdscr.addstr(row+2+(i*2),col+10,f"(Portals Available: {', '.join(portals)})",curses.color_pair(1)) #print the portals beside the room number to show what the portals goes to
            else: #check if unvisited
                self.stdscr.addstr(row+2+(i*2),col+2,f"Room: {r_id}",curses.A_DIM) #set that room id to default colour dimmed to indicate they have not reached that room yet
    #End of _render_minimap
    def render_screen(self): #render the entire game screen on the terminal for the player
        max_y,max_x=self.stdscr.getmaxyx() #find the maximum dimensions of the terminal window
        width,height=self.engine.get_room_dimensions() #find the dimensions of the current room
        minimap_height=len(self.engine.get_room_ids())*2 #find the height of the minimap
        required_width=max(75,width+minimap_height+25) #find the width thats necessary
        required_height=max(height+12,minimap_height+15) #find the height thats necessary
        if max_x<required_width or max_y<required_height: #check if the maximum terminal dimensions are less than the required dimensions
            raise RuntimeError("Terminal is too small. Please resize") #raise a RuntimeError to tell user to resize window in order to play the game
        self.stdscr.clear() #clear the standard screen to display a new frame
        self.stdscr.addstr(0,0,f"Message: {self.message}",curses.A_BOLD) #display the message bar at the very top
        room_id=self.engine.player.get_room() #retrieve the room id of the room the player is currently in
        self.stdscr.addstr(1,0,f"Room Number: {room_id}") #display the players current room id right below the message bar
        self._render_map() #call _render_map() helper function to display the entire room map on screen
        self._render_legend(width+5) #call _render_legend() helper function to display the legend on screen to the right of the room map
        self._render_minimap(3,width+35) #call _render_minimap() helper function to display the minimap on screen directly beside the legend
        bottom=max(height+3,minimap_height+8,10)+2 #set the bottom of the display dimensions to make sure it doesnt overlap with the map, legend, and minimap
        self._render_status(bottom) #call _render_status() helper function to display the status of the player
        self.stdscr.refresh() #refresh the screen to actually display everything as a new frame
    #End of render_screen
    def input_handling(self, key): #handle all inputs made by the player
        self.message="" #set the message bar to empty text
        move_inputs={
            ord('w'): Direction.NORTH, #set the 'w' key for going up
            ord('a'): Direction.WEST, #set the 'a' key for going left
            ord('s'): Direction.SOUTH, #set the 's' key for going down
            ord('d'): Direction.EAST, #set the 'd' key for going right
            curses.KEY_UP: Direction.NORTH,  #set the up arrow key for going up
            curses.KEY_LEFT: Direction.WEST, #set the left arrow key for going left
            curses.KEY_DOWN: Direction.SOUTH, #set the down arrow key for going down
            curses.KEY_RIGHT: Direction.EAST, #set the right arrow key for going right
        }#end of movement inputs
        try:
            if key in move_inputs: #check if user enters an input that is within the movement inputs
                self.engine.move_player(move_inputs[key]) #call move player Function and move player in accordance to direction key pressed
            elif key==ord('>'): #check if the key entered was the greater than symbol
                self.engine.access_portal() #call access portal function and shift player to the room that the portal connects to
                self.message="You have went through a portal" #update the message bar to tell the user they have entered a portal
            elif key==ord('r'): #check if the key entered was the 'r' key
                self.engine.reset() #reset the entire world
                self.message="World has been reset to beginning" #update message bar to tell user the world has resetted to it's initial state
            else: #check if key entered is an invalid input
                self.message="Invalid input" #display invalid input message on the message bar to tell user they have pressed an input that's not within the game controls
        except ImpassableError: #check if an impassable error occurs whilst player attempts to move
            self.message="You have bumped into a wall" #tell the player via message bar that they have bumped into a wall
        except Exception as error: #check if any other error occurs
            self.message=f"Error: {str(error)}" #display the error that occured on the message bar
    #End of input_handling
