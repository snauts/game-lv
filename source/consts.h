#ifndef __CONSTS_H__
#define __CONSTS_H__

const unsigned SCRIPT_USE_ITEM = 1;
const unsigned SCRIPT_SIMPLE = 2;
const unsigned SCRIPT_STARTUP = 3;
const unsigned SCRIPT_ENTER_SCENE = 4;
const unsigned SCRIPT_COMBINE_ITEM = 5;

const unsigned ACTION_DO_NOTHING = 0;
const unsigned ACTION_ADD_INVENTORY_ITEM = 1;
const unsigned ACTION_REMOVE_INVENTORY_ITEM = 2;
const unsigned ACTION_START_DIALOGUE = 3;
const unsigned ACTION_CHANGE_HOTSPOT_ANIMATION = 4;
const unsigned ACTION_DISPLAY_TEXT_STRING = 5;
const unsigned ACTION_SHOW_HOTSPOT = 6;
const unsigned ACTION_HIDE_HOTSPOT = 7;
const unsigned ACTION_START_NEW_SCENE = 8;
const unsigned ACTION_MOVE_CHARACTER = 9;
const unsigned ACTION_SET_CHARACTER_COORDINATES = 10;
const unsigned ACTION_ENABLE_USER_INPUT = 11;
const unsigned ACTION_DISABLE_USER_INPUT = 12;
const unsigned ACTION_SET_STATE = 13;
const unsigned ACTION_EXECUTE_SCRIPT_IF = 14;
const unsigned ACTION_STOP_SCRIPT_IF = 15;
const unsigned ACTION_STOP_SCRIPT_IFNOT = 16;
const unsigned ACTION_PLAY_CHARACTER_ANIMATION = 17;
const unsigned ACTION_SET_CHARACTER_DIRECTION = 18;
const unsigned ACTION_ENABLE_DIALOG_ANSWER = 19;
const unsigned ACTION_DISABLE_DIALOG_ANSWER = 20;
const unsigned ACTION_PLAY_MUSIC = 21;
const unsigned ACTION_STOP_MUSIC = 22;
const unsigned ACTION_PAUSE_MUSIC = 23;
const unsigned ACTION_RESUME_MUSIC = 24;
const unsigned ACTION_START_SOUND = 25;
const unsigned ACTION_PLAY_SOUND = 26;
const unsigned ACTION_STOP_SOUND = 27;
const unsigned ACTION_WAIT_TIME = 28;
const unsigned ACTION_PLAY_HOTSPOT_ANIMATION = 29;
const unsigned ACTION_SET_CHARACTER_ANIMATION_SET = 30;
const unsigned ACTION_SHOW_CHARACTER = 31;
const unsigned ACTION_HIDE_CHARACTER = 32;
const unsigned ACTION_MAP_CHARACTER_ANIMATION = 33;
const unsigned ACTION_UNMAP_CHARACTER_ANIMATION = 34;
const unsigned ACTION_ENABLE_GRAPH_LINE = 35;
const unsigned ACTION_DISABLE_GRAPH_LINE = 36;
const unsigned ACTION_TRIGGER_INTERRUPT_SCRIPT = 37;
const unsigned ACTION_TRIGGER_RESUME_SCRIPT = 38;
const unsigned ACTION_UNTRIGGER_SCRIPT = 39;
const unsigned ACTION_UNTRIGGER_ALL_SCRIPTS = 40;
const unsigned ACTION_CHANGE_DIALOG_QUESTION = 41;
const unsigned ACTION_CHANGE_DIALOG_ANSWER = 42;
const unsigned ACTION_PLAY_VIDEO = 43;
const unsigned ACTION_DO_FADE = 44;
const unsigned ACTION_CROSSFADE_MUSIC = 45;
const unsigned ACTION_DO_FADE_RANDOM = 46;
const unsigned ACTION_SHOW_GAME_CREDITS = 47;
const unsigned ACTION_END_CURRENT_GAME = 48;
const unsigned ACTION_WAIT_HOTSPOT_ANIM_END = 49;
const unsigned ACTION_SET_HOTSPOT_DEPTH = 50;
const unsigned ACTION_WAIT_HOTSPOT_ANIM_TRANSFORM_END = 51;
const unsigned ACTION_SKIP_ACTIONS_IF = 52;
const unsigned ACTION_CHAR_SAY_TEXT = 53;
const unsigned ACTION_NPC_SAY_TEXT = 54;
const unsigned ACTION_SET_SCRIPT_EXECUTION_COUNT = 55;

const unsigned CHARACTER_STAND = 1;
const unsigned CHARACTER_WALK = 2;
const unsigned CHARACTER_IDLE = 3;
const unsigned CHARACTER_TALK = 4;
const unsigned CHARACTER_ANIM = 5;

//const unsigned MAX_WALK_POINTS = 50;

const unsigned GAME_STATE_NORMAL = 1;
const unsigned GAME_STATE_NORMAL_DISABLED = 2;
const unsigned GAME_STATE_DIALOG = 3;
const unsigned GAME_STATE_DIALOG_DISABLED = 4;

const int ANY_INVENTORY_ITEM_ID = -1;

const int MUSIC_MODE_NEXT = 1;
const int MUSIC_MODE_RANDOM = 2;
const int MUSIC_MODE_STOP = 3;
const int MUSIC_MODE_RANDOM_STOP = 4;

const int SOUND_MODE_NEXT = 1;
const int SOUND_MODE_RANDOM = 2;

const double PI_COEF = 3.14159265354;

const unsigned max_text_items = 10;

const int DIALOG_SHOULD_START_DIALOG = 0;
const int DIALOG_SHOW_QUESTION = 1;
const int DIALOG_CHOOSE_ANSWER = 2;
const int DIALOG_SHOW_ANSWER = 3;
const int DIALOG_IN_QUESTION_SCRIPT = 4;
const int DIALOG_IN_ANSWER_SCRIPT = 5;
const int DIALOG_OUT_QUESTION_SCRIPT = 6;
const int DIALOG_OUT_ANSWER_SCRIPT = 7;


const unsigned SUSPENDABLE_ACTIONS = 8;
const unsigned SCRIPT_ACTION_STATIC_DATA_SIZE = 5;

const int SCRIPT_TRIGGER_INTERRUPT = 0;
const int SCRIPT_TRIGGER_RESUME = 1;

const int MUSIC_FADE_OUT_AND_PAUSE = 0;
const int MUSIC_RESUME_AND_FADE_IN = 1;
const int MUSIC_FADE_TO_VOLUME = 2;
const int MUSIC_CROSSFADE_SONGS = 3;

const int INVENTORY_NOTHING = 0;
const int INVENTORY_USE_ITEM = 1;
const int INVENTORY_COMBINE_ITEM = 2;
const int INVENTORY_MOVE_CHARACTER = 3;

enum {
	GAME_EVENT_END_CURRENT_GAME = SDL_USEREVENT + 1
};

enum QUESTION_SELECT_MODE {
	QUESTION_SELECT_FIRST,
		QUESTION_SELECT_LOOP,
		QUESTION_SELECT_RANDOM,
		QUESTION_SELECT_LIST
};

enum ANSWER_SELECT_MODE {
	ANSWER_SELECT_FIRST,
		ANSWER_SELECT_LOOP,
		ANSWER_SELECT_RANDOM,
		ANSWER_SELECT_LIST 
};

enum HOTSPOT_SHOW_MODE {
	HOTSPOT_RESET_ANIM_RESTART_SOUND = 0, // Reset animation and start playing sound from beginning
		HOTSPOT_UPDATE_ANIM_NO_SOUND, // Update animation, but dont play sound
		HOTSPOT_UPDATE_ANIM_RESTART_SOUND, // Update animation and play sound from beginning
		HOTSPOT_SHOW_MODE_INVALID
};

const int TRANSFORM_LINEAR = 1;
const int TRANSFORM_SPLINE = 2;


#endif
