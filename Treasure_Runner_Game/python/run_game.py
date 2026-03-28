import os
import argparse
import json
import curses
from datetime import datetime
from treasure_runner.models.game_engine import GameEngine
from treasure_runner.ui.game_ui import GameUI
#The curses implementation in this assignment is used from CIS*2750 lectures from MVC_Curses (March 12 2026), and More_Curses (March 17 2026)
def parse_args():
    parser = argparse.ArgumentParser(description="Treasure Runner Game")
    parser.add_argument( #define the configuration path argument
        "--config",
        required=True,
        help="Path to generator config file",
    )
    parser.add_argument( #define the profile path argument
        "--profile",
        required=True,
        help="Output profile path",
    )
    return parser.parse_args()
#End of parse_args
def main():
    args = parse_args() #call parse_args() to fetch the parsed arguements
    config_path = os.path.abspath(args.config) #convert configuration path to absolute
    profile_path = os.path.abspath(args.profile) #convert profile path to absolute
    print(f"Loading world from: {config_path}") #print message on terminal to tell user the program is loading world from the configuration path given
    if os.path.exists(profile_path): #check if profile path exists
        print(f"Found profile from {profile_path} ") #print message on terminal to tell user the program found a profile from given profile path
        with open(profile_path, 'r') as file: #open file in read mode
            player_profile = json.load(file) #load JSON data into player_profile
    else:#if there is not profile path
        print(f"Unable to find profile at {profile_path}") #print message that profile path does not contain profile
        player_name = input("Enter player name to create profile: ") #prompt the user to enter a name for their player to create profile
        player_profile = {
            "player_name": player_name, #set the player's name to the name entered by user
            "games_played": 0, #set the amount of games played to zero
            "max_treasure_collected": 0, #set the maximum amount of treasures collected to zero
            "most_rooms_world_completed": 0, #set the biggest world completed to 0
            "timestamp_last_played": datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%SZ') #set the timestamp last played to the current time in UTC
        }
        with open(profile_path,'w') as file: #open the file in write mode
            json.dump(player_profile,file,indent=4) #dump JSON file into player profile
        print(f"Created profile from {profile_path}") #print message that profile has been created from profile path given
    player_profile['games_played']+=1 #increment the amount of games played by player
    player_profile['timestamp_last_played']=datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%SZ') #update the time that the user last played
    try:
        engine=GameEngine(config_path) #pass configuration file to python wrapper
    except Exception as e: #catch exception if C rejects the config file
        print(f"Error initializing game engine: {e}") #print error message indiicating game engine could not be initialized
        return 1 #exit program by returning 1
    def run_curses(stdscr): #used to accapt stdscr for UI
        ui=GameUI(stdscr,engine,player_profile,profile_path) #call the GameUI class with the stdscr, engine, player_profile and profile_path
        ui.run() #run the game
    try:
        curses.wrapper(run_curses) #use curses.wrapper to handle the terminal screen
    finally:
        engine.destroy() #destroy the engine to free it from memory
    return 0 #return zero to indicate success
    #End of run_curses
#End of main
if __name__ == "__main__":
    raise SystemExit(main())
