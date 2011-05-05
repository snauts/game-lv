#ifndef __GAME_STRINGS_H__
#define __GAME_STRINGS_H__




// Latvian language strings

#ifdef LATVIAN

#define STRING_MENU_NEW_GAME "Jauna spçle"
#define STRING_MENU_SAVE_GAME "Saglabât spçli"
#define STRING_MENU_LOAD_GAME "Ielâdçt spçli"
#define STRING_MENU_RESUME_GAME "Turpinât spçli"
#define STRING_MENU_CREDITS "Veidotâji"
#define STRING_MENU_EXIT_GAME "Beigt"
#define STRING_MENU_NAME_MAIN "Galvenâ izvçlne"
#define STRING_MENU_EXIT_YES "Jâ"
#define STRING_MENU_EXIT_NO "Nç"
#define STRING_MENU_REALLY_EXIT "Iziet no spçles?"
#define STRING_SLOT_EMPTY "<Tukðs>"
#define STRING_MENU_CANCEL "Atcelt"
#define STRING_SELECT_SAVEGAME "Saglabât spçli"
#define STRING_SELECT_LOADGAME "Ielâdçt spçli"

#define STRING_STARTING_NEW_GAME "Notiek jaunas spçles ielâde"
#define STRING_LOADING_GAME "Notiek spçles ielâde"
#define STRING_GAME_INITIALIZING "Spçles inicializâcija, lûdzu uzgaidiet"

#define STRING_MENU_OVERWRITE_GAME "Pârrakstît pâri paðreiz saglabâtajai spçlei: %s?"
#define STRING_MENU_OVERWRITE_YES "Jâ"
#define STRING_MENU_OVERWRITE_NO "Nç"

#define STRING_SAVEGAME_NAME "Ievadiet saglabâjamâs spçles nosaukumu:"
#define STRING_MENU_CANCEL_SAVEGAME "Atcelt"
#define STRING_EMPTY_GAMESLOT_NAME "Jauna"
#define STRING_NONAMED_SAVEGAME "<Nav nosaukums>"

// End of latvian

#else 

// Default language- english strings
#define STRING_MENU_NEW_GAME "Start new game"
#define STRING_MENU_SAVE_GAME "Save game"
#define STRING_MENU_LOAD_GAME "Load game"
#define STRING_MENU_RESUME_GAME "Resume game"
#define STRING_MENU_CREDITS "Credits"
#define STRING_MENU_EXIT_GAME "Exit game"
#define STRING_MENU_NAME_MAIN "Main menu"
#define STRING_MENU_EXIT_YES "Yes"
#define STRING_MENU_EXIT_NO "No"
#define STRING_MENU_REALLY_EXIT "Leave the game?"
#define STRING_SLOT_EMPTY "<Empty>"
#define STRING_MENU_CANCEL "Cancel"
#define STRING_SELECT_SAVEGAME "Select savegame slot"
#define STRING_SELECT_LOADGAME "Select game to load"

#define STRING_STARTING_NEW_GAME "Starting new game"
#define STRING_LOADING_GAME "Loading game"
#define STRING_GAME_INITIALIZING "Initializing game, please wait"

#define STRING_MENU_OVERWRITE_GAME "Overwrite currently saved game: %s?"
#define STRING_MENU_OVERWRITE_YES "Yes"
#define STRING_MENU_OVERWRITE_NO "No"

#define STRING_SAVEGAME_NAME "Enter name for saved game:"
#define STRING_MENU_CANCEL_SAVEGAME "Cancel"
#define STRING_EMPTY_GAMESLOT_NAME "New"
#define STRING_NONAMED_SAVEGAME "<No name>"

#endif // Language selection

#endif
