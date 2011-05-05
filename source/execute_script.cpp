#include "game.h"
#include "consts.h"
#include "structs.h"
#include "sound.h"

int do_nothing(ScriptAction* a);
int add_inventory_item(ScriptAction* a);
int remove_inventory_item(ScriptAction* a);
int start_dialogue(ScriptAction* a);
int change_hotspot_animation(ScriptAction* a);
int display_text_string(ScriptAction* a);
int show_hotspot(ScriptAction* a);
int hide_hotspot(ScriptAction* a);
int start_new_scene(ScriptAction* a);
int move_character(ScriptAction* a);
int set_character_coordinates(ScriptAction* a);
int enable_user_input(ScriptAction* a);
int disable_user_input(ScriptAction* a);
int set_game_state(ScriptAction* a);
int execute_script_if_state(ScriptAction* a);
int stop_script_if(ScriptAction* a);
int stop_script_ifnot(ScriptAction* a);
int play_character_animation(ScriptAction* a);
int set_character_direction(ScriptAction* a);
int enable_dialog_answer(ScriptAction* a);
int disable_dialog_answer(ScriptAction* a);
int start_music(ScriptAction* a);
int stop_music(ScriptAction* a);
int pause_music(ScriptAction* a);
int resume_music(ScriptAction* a);
int start_sound(ScriptAction *a);
int play_sound(ScriptAction *a);
int stop_sound(ScriptAction *a);
int wait_time(ScriptAction *a);
int play_hotspot_animation(ScriptAction *a);
int set_character_animation_set(ScriptAction *a);
int show_character(ScriptAction *a);
int hide_character(ScriptAction *a);
int map_character_animation(ScriptAction *a);
int unmap_character_animation(ScriptAction *a);
int enable_graph_line(ScriptAction *a);
int disable_graph_line(ScriptAction *a);
int trigger_interrupt_script(ScriptAction *a);
int trigger_resume_script(ScriptAction *a);
int untrigger_script(ScriptAction *a);
int untrigger_all_scripts(ScriptAction *a);
int change_dialog_question(ScriptAction *a);
int change_dialog_answer(ScriptAction *a);
int play_video(ScriptAction *a);
int do_fade(ScriptAction *a);
int crossfade_music(ScriptAction *a);
int do_fade_random(ScriptAction *a);
int show_game_credits(ScriptAction *a);
int end_current_game(ScriptAction *a);
int wait_hotspot_anim_end(ScriptAction *a);
int set_hotspot_depth(ScriptAction *a);
int wait_hotspot_anim_transform_end(ScriptAction *a);
int skip_actions_if(ScriptAction *a);
int char_say_text(ScriptAction *a);
int npc_say_text(ScriptAction *a);
int set_script_execution_count(ScriptAction *a);

int resumeScript(Script* s, ScriptAction* act, bool skip_this_action) {
	if(skip_this_action) {
		dout << "--- Resumed script: " << s->id << endl << endl;
	}
	ScriptAction* action;
	bool was_found = false;
	unsigned retCode = 0;
	for(int a = 0; a < s->actions.size(); a++) {
		action = s->actions[a];
		if(!was_found) {
			if(action != act) {
				continue;
			} 
			else {
				was_found = true;
				if(skip_this_action)
					continue;
			}
		}
		dout << " executing action: " << action->action << endl;
		switch(action->action) {
		case(ACTION_DO_NOTHING) :
			{
				do_nothing(action);
				break;
			}
		case(ACTION_ADD_INVENTORY_ITEM) :
			{
				add_inventory_item(action);
				break;
			}
		case(ACTION_REMOVE_INVENTORY_ITEM) :
			{
				remove_inventory_item(action);
				break;
			}
		case(ACTION_START_DIALOGUE) :
			{
				int ret = start_dialogue(action);
				if(ret > 0) {
					// all ok, dialog started
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this means error.
					// but anyway, lets continue script.
				}
				break;
			}
		case(ACTION_CHANGE_HOTSPOT_ANIMATION) :
			{
				change_hotspot_animation(action);
				break;
			}
		case(ACTION_DISPLAY_TEXT_STRING) :
			{
				display_text_string(action);
				break;
			}
		case(ACTION_SHOW_HOTSPOT) :
			{
				show_hotspot(action);
				break;
			}
		case(ACTION_HIDE_HOTSPOT) :
			{
				hide_hotspot(action);
				break;
			}
		case(ACTION_START_NEW_SCENE) :
			{
				start_new_scene(action);
				break;
			}
		case(ACTION_MOVE_CHARACTER) :
			{
				int ret = move_character(action);
				if(ret > 0) {
					// all ok, character is moving.
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else if(ret == 0) {
					// character is already on the spot.
					// script continues
				} else {
					// this means error.
					// most likely spot is unreachable.
					// so we won't continue script.
					dout << "WARNING: Could not go to the selected point: " << action->param1 
						<< "," << action->param1 << " script " << s->id << " won't continue." << endl;
					a = s->actions.size();
				}
				break;
			}
		case(ACTION_SET_CHARACTER_COORDINATES) :
			{
				set_character_coordinates(action);
				break;
			}
		case(ACTION_ENABLE_USER_INPUT) :
			{
				enable_user_input(action);
				s->execute_times--;
				break;
			}
		case(ACTION_DISABLE_USER_INPUT) :
			{
				disable_user_input(action);
				break;
			}
		case(ACTION_SET_STATE) :
			{
				set_game_state(action);
				break;
			}
		case(ACTION_STOP_SCRIPT_IF) :
			{
				if(stop_script_if(action))				
					a = s->actions.size();						
				break;
			}
		case(ACTION_STOP_SCRIPT_IFNOT) :
			{
				if(stop_script_ifnot(action))				
					a = s->actions.size();						
				break;
			}
		case(ACTION_EXECUTE_SCRIPT_IF) :
			{
				if(execute_script_if_state(action)) 
					a = s->actions.size();
				break;
			}
		case(ACTION_PLAY_CHARACTER_ANIMATION) :
			{
				int ret = play_character_animation(action);
				if(ret > 0) {
					// all ok, character animation is playing.
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this means error.
					// but anyway, lets continue script.
				}
				break;
			}
		case(ACTION_SET_CHARACTER_DIRECTION) :
			{
				set_character_direction(action);
				break;
			}
		case ACTION_ENABLE_DIALOG_ANSWER:
			{
				enable_dialog_answer(action);
				break;
			}
		case ACTION_DISABLE_DIALOG_ANSWER:
			{
				disable_dialog_answer(action);
				break;
			}
		case ACTION_PLAY_MUSIC:
			{
				start_music(action);
				break;
			}
		case ACTION_STOP_MUSIC:
			{
				stop_music(action);
				break;
			}
		case ACTION_PAUSE_MUSIC:
			{
				pause_music(action);
				break;
			}
		case ACTION_RESUME_MUSIC:
			{
				resume_music(action);
				break;
			}
		case ACTION_START_SOUND:
			{
				start_sound(action);
				break;
			}
		case ACTION_PLAY_SOUND:
			{
				int ret = play_sound(action);
				if(ret > 0) {
					// all ok, sound is playing.
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this means error.
					// but anyway, lets continue script.
				}
				break;
			}
		case ACTION_STOP_SOUND:
			{
				stop_sound(action);
				break;
			}
		case ACTION_WAIT_TIME:
			{
				int ret = wait_time(action);
				if(ret > 0) {
					// all ok, waiting started
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this means error.
					// but anyway, lets continue script.
				}
				break;
			}
		case ACTION_PLAY_HOTSPOT_ANIMATION:
			{
				int ret = play_hotspot_animation(action);
				if(ret > 0) {
					// all ok, animation started
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this means error.
					// but anyway, lets continue script.
				}
				break;
			}
		case ACTION_SET_CHARACTER_ANIMATION_SET:
			{
				set_character_animation_set(action);
				break;
			}
		case ACTION_SHOW_CHARACTER:
			{
				show_character(action);
				break;
			}
		case ACTION_HIDE_CHARACTER:
			{
				hide_character(action);
				break;
			}
		case ACTION_MAP_CHARACTER_ANIMATION:
			{
				map_character_animation(action);
				break;
			}
		case ACTION_UNMAP_CHARACTER_ANIMATION:
			{
				unmap_character_animation(action);
				break;
			}
		case ACTION_ENABLE_GRAPH_LINE:
			{
				enable_graph_line(action);
				break;
			}
		case ACTION_DISABLE_GRAPH_LINE:
			{
				disable_graph_line(action);
				break;
			}
		case ACTION_TRIGGER_INTERRUPT_SCRIPT:
			{
				trigger_interrupt_script(action);
				break;
			}
		case ACTION_TRIGGER_RESUME_SCRIPT:
			{
				trigger_resume_script(action);
				break;
			}
		case ACTION_UNTRIGGER_SCRIPT:
			{
				untrigger_script(action);
				break;
			}
		case ACTION_UNTRIGGER_ALL_SCRIPTS:
			{
				untrigger_all_scripts(action);
				break;
			}
		case ACTION_CHANGE_DIALOG_QUESTION:
			{
				change_dialog_question(action);
				break;
			}
		case ACTION_CHANGE_DIALOG_ANSWER:
			{
				change_dialog_answer(action);
				break;
			}
		case ACTION_PLAY_VIDEO:
			{
				play_video(action);
				break;
			}
		case ACTION_DO_FADE:
			{
				do_fade(action);
				break;
			}
		case ACTION_CROSSFADE_MUSIC:
			{
				crossfade_music(action);
				break;
			}
		case ACTION_DO_FADE_RANDOM:
			{
				do_fade_random(action);
				break;
			}
		case ACTION_SHOW_GAME_CREDITS:
			{
				show_game_credits(action);
				break;
			}
		case ACTION_END_CURRENT_GAME:
			{
				end_current_game(action);
				a = s->actions.size();
				break;
			}
		case ACTION_WAIT_HOTSPOT_ANIM_END:
			{
				int ret = wait_hotspot_anim_end(action);
				if(ret > 0) {
					// all ok, waiting started 
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this could mean error.
				}
				break;
			}
		case ACTION_SET_HOTSPOT_DEPTH:
			{
				set_hotspot_depth(action);
				break;
			}
		case ACTION_WAIT_HOTSPOT_ANIM_TRANSFORM_END:
			{
				int ret = wait_hotspot_anim_transform_end(action);
				if(ret > 0) {
					// all ok, waiting started 
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// strange... this could mean error.
				}
				break;
			}
		case ACTION_SKIP_ACTIONS_IF:
			{
				int ret = skip_actions_if(action);
				if(ret) {
					a += ret;
				}
				break;
			}
		case ACTION_CHAR_SAY_TEXT:
			{
				int ret = char_say_text(action);
				if(ret > 0) {
					// all ok, dialog started
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// lets continue script.
				}
				break;
			}
		case ACTION_NPC_SAY_TEXT:
			{
				int ret = npc_say_text(action);
				if(ret > 0) {
					// all ok, dialog started
					retCode = 1; // return that script is suspended.
					a = s->actions.size();
					main_character->suspended_script = s;
					main_character->suspended_action = action;
				} else {
					// lets continue script.
				}
				break;
			}
		case ACTION_SET_SCRIPT_EXECUTION_COUNT:
			{
				set_script_execution_count(action);
				break;
			}
		default:
			{
				dout << " unknown action: " << action->action << endl;
				break;
			}
		}
	}
	if(skip_this_action) {
		dout << "--- Resumed script ended" << endl << endl ;
	}

	int tmp = 0;

	for(tmp = 0; tmp < s->actions.size(); tmp++) {
		action = s->actions[tmp];
		if(action->action == ACTION_ENABLE_USER_INPUT) {
			break;
		}
	}
	if(tmp == s->actions.size()) {
		s->execute_times--;
	}
	return retCode;
}

int executeScript(Script* s) {
	dout << endl << "--- Executing script: " << s->id << endl;
	if(s->execute_times <= 0)
		return 1;
	if(s->actions.size() > 0) {
		ScriptAction* action = s->actions[0];
		resumeScript(s, action);
	}
	dout << "--- Script executed" << endl << endl;
	return 0;	
}

int do_nothing(ScriptAction* a){ 
	dout << "  do nothing" << endl;
	return 0; 
}
int add_inventory_item(ScriptAction* a) { 
	dout << "  add inventory item" << endl;
	IIMI i = all_inventory_items.find(a->param1);
	if(i == all_inventory_items.end()) {
		dout << "ERROR- add_inventory_item refers to nonexisting item: " << a->param1 << endl;
		exit(1);
	}
	InventoryItem* inv = i->second;
	if(inventory.find(a->param1) != inventory.end()) {
		dout << "   add inventory item: " << a->param1 << " (" << inv->short_name << ") -already in inventory" << endl;
		return 0;
	}
	LoadInventoryItemData(inv);
	inventory.insert(make_pair<unsigned, InventoryItem*>(inv->id, inv));
	currentInventorySkin->onAddItem(inv->id);
	dout << "item: " << a->param1 <<" (" << inv->short_name << ") added to inventory" << endl;
	return 0; 
}
int remove_inventory_item(ScriptAction* a){
	dout << "  remove inventory item" << endl;
	IIMI i = inventory.find(a->param1);
	if(i == inventory.end()) {
		dout << "   item: " << a->param1 << " is not in inventory" << endl;
	} else {
		InventoryItem* inv = i->second;
		inventory.erase(i);
		currentInventorySkin->onRemoveItem(inv->id);
		FreeInventoryItemData(inv);
		dout << "   item: " << inv->id << " (" << inv->short_name << ") removed from inventory" << endl;
	}
	return 0; 
}
int start_dialogue(ScriptAction* a) { 
	dout << "  start dialog: " << a->param1 << endl;
	return suspendScript(a);
}
int change_hotspot_animation(ScriptAction* a) { 
	dout << "  change hotspot animation" << endl;
	int hotspot_id = a->param1;
	int anim_id = a->param2;
	HMI i = all_hotspots.find(hotspot_id);
	if(i == all_hotspots.end()) {
		dout << "ERROR- unknown hotspot: " << hotspot_id << endl;
		exit(1);
//		return 1;
	}
	Hotspot* h = i->second;
	SetHotspotAnim(h, anim_id);
	soundPlayer.playHotspotSound(h->id, h->sound);
	return 0;
}
int display_text_string(ScriptAction* a) { 
	dout << "  draw text: '" << a->str << "'" << endl;
	TextItem i;
	i.text = a->str;
	i.expireTime = current_time + a->param1;
	i.fontId = a->param2;
	FMI fmi = allFonts.find(i.fontId);
	if(fmi == allFonts.end()) {
		dout << "ERROR- Font with id: " << i.fontId << " does not exist." << endl;
		exit(1);
	}
	i.font = fmi->second;
	onscreen_text.push_back(i);
	should_draw_text = true;
	while(onscreen_text.size() > max_text_items)
		onscreen_text.pop_front();
	return 0; 
}
int show_hotspot(ScriptAction* a){ 
	dout << "  show hotspot" << endl;
	HMI i = all_hotspots.find(a->param1);
	if(i != all_hotspots.end()) {
		Hotspot* h = i->second;
		if(!h->enabled) {
			dout << "   showing hotspot: " << h->id << " (" << h->description << ")" << endl;
			h->enabled = true;
			// Check whether hotspot is in this scene
			if(current_scene->hotspots.find(h->id) != current_scene->hotspots.end()) {
				if(h->anim) {
					switch(h->showModeEnableHotspot) {
					case HOTSPOT_RESET_ANIM_RESTART_SOUND:
						{
							resetAnimation(h->anim);
							break;
						}
					case HOTSPOT_UPDATE_ANIM_RESTART_SOUND:
					case HOTSPOT_UPDATE_ANIM_NO_SOUND:
						{
							UpdateAnimation(h->anim);
							break;
						}
					}
				}
				switch(h->showModeEnableHotspot) {
				case HOTSPOT_RESET_ANIM_RESTART_SOUND:
				case HOTSPOT_UPDATE_ANIM_RESTART_SOUND:
					{
						soundPlayer.playHotspotSound(h->id, h->sound);
						break;
					}
				}
				ResortHotspots(current_scene);
			}
		}
		else {
			dout << "   hotspot: " << h->id  << " (" << h->description << ") already visible" << endl;
		}
	} 
	return 0;
}
int hide_hotspot(ScriptAction* a) {
	dout << "  hide hotspot" << endl;
	HMI i = all_hotspots.find(a->param1);
	if(i != all_hotspots.end()) {
		Hotspot* h = i->second;
		if(h->enabled) {
			dout << "   hiding hotspot: " << h->id << " (" << h->description << ")" << endl;
			h->enabled = false;
			// Check whether hotspot is in this scene
			if(current_scene->hotspots.find(h->id) != current_scene->hotspots.end()) {
				ResortHotspots(current_scene);
				soundPlayer.stopHotspotSound(h->id);
			}
		}
		else {
			dout << "   hotspot: " << h->id  << " (" << h->description << ") already hidden" << endl;
		}
	} 
	return 0;
}

int start_new_scene(ScriptAction* a) { 
	dout << "  start scene: " << a->param1 << endl;
	int id = a->param1;
	SCMI i = all_scenes.find(id);
	Hotspot *h;
	if(i == all_scenes.end()) {
		dout << "ERROR- start_new_scene: could not find scene with id: " << id << endl;
		exit(1);
	}

	stopTime();

	Scene* s = i->second;
	Scene* prevScene = current_scene;
	// If old and new scene have equal sound files, sound should not be restarted
	bool sceneSoundsEqual = false; 
	
	if(s == current_scene) {
		return 0;
	} else {

		if(current_scene) {
			if(current_scene->gfx)
				delete current_scene->gfx;

			sceneSoundsEqual = (s->soundFile == current_scene->soundFile && s->sound);

			if(sceneSoundsEqual) {
				s->sound = current_scene->sound;
			} else {
				soundPlayer.stopSceneSound();
				if(current_scene->sound)
					delete current_scene->sound;
			}
			soundPlayer.stopAllHotspotSound();
			
			current_scene->walkMask->freeData();

			current_scene->gfx = NULL;
			current_scene->sound = NULL;

			HMI hmi = current_scene->hotspots.begin();
			while(hmi != current_scene->hotspots.end()) {
				h = hmi->second;
				freeHotspotAnimData(h);
				hmi++;
			}
			if(sceneSoundsEqual) {
				soundPlayer.freeSoundData(SOUND_TYPE_DIALOG | SOUND_TYPE_HOTSPOT);
			} else {
				soundPlayer.freeSoundData(SOUND_TYPE_SCENE | SOUND_TYPE_DIALOG | SOUND_TYPE_HOTSPOT);
			}
		}
	}
	s->gfx = LoadAnimation(s->gfxFile);
	if(!s->gfx) {
		dout << "Could not load gfx for scene: " << s->id << endl;
		exit(1);
	}

	if(s->walkMask->loadData()) {
		dout << "ERROR- while loading walkmask: " << s->maskFile << endl;
		exit(1);
	}
	s->w = s->walkMask->getWidth();
	s->h = s->walkMask->getHeight();

	if(!sceneSoundsEqual) {
		s->sound = loadSoundCollection(s->soundFile, SOUND_TYPE_SCENE);
	}

	current_scene = s;

	HMI hmi = current_scene->hotspots.begin();
	while(hmi != current_scene->hotspots.end()) {
		h = hmi->second;
		loadHotspotAnimData(h);
		switch(h->showModeEnterScene) {
		case HOTSPOT_RESET_ANIM_RESTART_SOUND:
			{
				resetAnimation(h->anim);
				break;
			}
		case HOTSPOT_UPDATE_ANIM_NO_SOUND:
		case HOTSPOT_UPDATE_ANIM_RESTART_SOUND:
			{
				UpdateAnimation(h->anim);
				break;
			}
		}
		hmi++;
	}
				
	if(!sceneSoundsEqual) {
		soundPlayer.playSceneSound(s->sound);
	}

	HMI hotspotIter = s->hotspots.begin();
	while(hotspotIter != s->hotspots.end()) {
		h = hotspotIter->second;
		if(h->enabled) {
			switch(h->showModeEnterScene) {
			case HOTSPOT_RESET_ANIM_RESTART_SOUND:
			case HOTSPOT_UPDATE_ANIM_RESTART_SOUND:
				{
					soundPlayer.playHotspotSound(h->id, h->sound);
					break;
				}
			}
		}
		hotspotIter++;
	}

	ResortHotspots(current_scene);

	resumeTime();
	
	if(a->param2 == 1) {
		dout << "    scene startup scripts ignored" << endl;
	} else {
		executeSceneStartupScripts(prevScene, current_scene);
	}

	dout << "scene: " << s->id << " started" << endl;
	return 0; 
}

int move_character(ScriptAction* a) { 
	dout << "  move character" << endl;
	return suspendScript(a);
}

int set_character_coordinates(ScriptAction* a){ 
	dout << "  set character coordinates" << endl;
	int x = a->param1;
	int y = a->param2;
	if(!main_character) {
		dout << "ERROR- there is no character in game" << endl;
		exit(1);
//		return 1;
	}
	if(!current_scene) {
		dout << "ERROR- there is no scene loaded" << endl;
		exit(1);
//		return 1;
	} 
	else {
		if(x >= current_scene->w) {
			dout << "ERROR- x position is out of range" << endl;
			exit(1);
//			return 1;
		}
		if(y >= current_scene->h) {
			dout << "ERROR- y position is out of range" << endl;
			exit(1);
//			return 1;
		}
		if(!current_scene->walkMask->isOnMask(x, y)) {
			dout << "ERROR- mask is not walkable in point: " << x << ", " << y << endl;
			exit(1);
//			return 1;
		}
		main_character->coord.x = x;
		main_character->coord.y = y;
		
		current_scene->walkMask->resetPath(main_character->coord);
		setCharacterState(CHARACTER_STAND);
		SetCurrentCharacterAnimation(main_character);
	}
	dout << "character position set to: " << x << ", " << y << endl;
	return 0; 
}

int enable_user_input(ScriptAction* a) {
	dout << "  enabling user input" << endl;
	switch(current_game_state) {
	case GAME_STATE_NORMAL_DISABLED:
		{
			enterGameState(GAME_STATE_NORMAL);
			break;
		}
	case GAME_STATE_DIALOG_DISABLED:
		{
			enterGameState(GAME_STATE_DIALOG, state_var1, state_var2);
			break;
		}
	}
	return 0;
}
int disable_user_input(ScriptAction* a) {
	dout << "  disabling user input" << endl;
	switch(current_game_state) {
	case GAME_STATE_NORMAL:
		{
			enterGameState(GAME_STATE_NORMAL_DISABLED);
			break;
		}
	case GAME_STATE_DIALOG:
		{
			enterGameState(GAME_STATE_DIALOG_DISABLED, state_var1, state_var2);
			break;
		}
	}
	return 0;
}

int set_game_state(ScriptAction* a) {
	dout << "  setting game state" << endl;
	dout << "   name: '" << a->str << "', value : " << a->param1 << endl;
	game_states.set_value(a->str, a->param1);
	return 0;
}

int execute_script_if_state(ScriptAction* a) {
	dout << "  execute script: " << a->param2 << " if '" << a->str << "' = " << a->param1 << endl;
	dout << "   " << a->str << " : " << game_states.get_value(a->str) << endl;
	if(game_states.get_value(a->str) == a->param1) {
		executeScriptById(a->param2);
		return 1;
	}
	return 0;
}
int stop_script_if(ScriptAction* a) {
	dout << "  stop script if '" << a->str << "' = " << a->param1 << endl;
	if(game_states.get_value(a->str) == a->param1)
		return 1;
	return 0;
}
int stop_script_ifnot(ScriptAction* a) {
	dout << "  stop script if '" << a->str << "' <> " << a->param1 << endl;
	if(game_states.get_value(a->str) != a->param1)
		return 1;
	return 0;
}

int play_character_animation(ScriptAction* a) {
	dout << "  play character animation: " << a->param1 << endl;
	dout << " started in: " << current_time << endl;
	return suspendScript(a);
}

int set_character_direction(ScriptAction* a) {
	dout << "  set character direction" << endl;
	if(main_character->state == CHARACTER_STAND) {
		double deg = a->param1 % 360;
		double rad = (deg * PI_COEF) / 180.0;
		main_character->coord.angle = rad;
		SetCurrentCharacterAnimation(main_character);
	}
	else {
		dout << "WARNING: character is not standing, state is: " << main_character->state << endl;
	}
	return 0;
}

int set_answer_state(int dialogId, int answerId, bool enabled) {
	dout << "  set answer state" << endl;
	DMI i = all_dialogs.find(dialogId);
	Dialog *d;
	if(i == all_dialogs.end()) {
		dout << "ERROR- Could not find dialog: " << dialogId << endl;
		exit(1);
//		return 1;
	}
	d = i->second;
	map<int, DialogQuestion>::iterator questionIterator = d->questions.begin();
	int count;
	while(questionIterator != d->questions.end()) {
		DialogQuestion &question = questionIterator->second;
		for(count = 0; count < question.answers.size(); count++) {
			DialogAnswer &answer = question.answers[count];
			if(answer.id == answerId) {
				if(answer.enabled != enabled) {
					if(enabled) {
						question.enabledAnswers++;
					} else {
						question.enabledAnswers--;
					}
				}
				dout << " dialog answer: '" << answer.answerChoose[0] << "' found" << endl;
				answer.enabled = enabled;
				return 0;
			}
		}
		questionIterator++;
	}
	dout << "ERROR- Could not find answer with id:" << answerId << endl;
	exit(1);
	return 1;
}

int enable_dialog_answer(ScriptAction* a) {
	dout << "  enable dialog answer" << endl;
	return set_answer_state(a->param1, a->param2, true);
}
int disable_dialog_answer(ScriptAction* a) {
	dout << "  disable dialog answer" << endl;
	return set_answer_state(a->param1, a->param2, false);
}

int start_music(ScriptAction* a) {
	dout << "  start music" << endl;
	return musicPlayer.playMusic(a->param1, a->param2);
}
int stop_music(ScriptAction* a) {
	dout << "  stop music" << endl;
	return musicPlayer.stopMusic();
}
int pause_music(ScriptAction* a) {
	dout << "  pause music" << endl;
	return musicPlayer.pauseMusic();
}
int resume_music(ScriptAction* a) {
	dout << "  resume music" << endl;
	return musicPlayer.resumeMusic();
}

int wait_time(ScriptAction *a) {
	dout << "  wait for some time: " << a->param1 << "/100sec" << endl;
	return suspendScript(a);
}
int start_sound(ScriptAction *a) {
	dout << "  start script sound: " << a->param2 << " on channel: " << a->param1 << endl;
	map<int, SoundCollection*>::iterator i = scriptSounds.find(a->param2);
	SoundCollection *s;
	if(i == scriptSounds.end()) {
		dout << "ERROR- could not find script sound with id: " << a->param2 << endl;
		exit(1);
//		return 1;
	}
	s = i->second;
	soundPlayer.playScriptSound(a->param1, s);
	return 0;
}
int play_sound(ScriptAction *a) {
	dout << "  play sound" << endl;
	if(!start_sound(a)) {
		return suspendScript(a);
	}
	return 0;
}
int stop_sound(ScriptAction *a) {
	dout << "  stop script sound, channel: " << a->param1 << endl;
	soundPlayer.stopScriptSound(a->param1);
	return 0;
}

int play_hotspot_animation(ScriptAction *a) {
	dout << "  play hotspot animation, hotspot: " << a->param1 << " animation id: " << a->param2 << endl;
	return suspendScript(a);
}
int set_character_animation_set(ScriptAction *a) {
	dout << "  set character animation set: " << a->param1 << endl;
	unsigned err;
	stopTime();
	err = setCharacterAnimationSet(a->param1);
	resumeTime();
	return err;
}

int show_character(ScriptAction *a) {
	dout << "  show character" << endl;
	main_character->visible = true;
	SetCurrentCharacterAnimation(main_character);
	return 0;
}
int hide_character(ScriptAction *a) {
	dout << "  hide character" << endl;
	main_character->visible = false;
	return 0;
}

int map_character_animation(ScriptAction *a) {
	dout << "  map character animation- unmap first." << endl;
	unmap_character_animation(a);
	dout << "  map character animation: " << a->param1 << " to " << a->param2 << endl;
	main_character->mappedAnimationIds.insert(make_pair<int, int>(a->param1, a->param2));

	map<int, CharacterAnimationSet*>::iterator ci = 
		main_character->animationSet.find(main_character->currentAnimationSetId);
	CharacterAnimationSet *set = ci->second;
	set->mappedAnimationIds = main_character->mappedAnimationIds;

	SetCurrentCharacterAnimation(main_character, true);
	return 0;
}
int unmap_character_animation(ScriptAction *a) {
	dout << "  unmap character animation: " << a->param1 << endl;
	map<int, int>::iterator i = main_character->mappedAnimationIds.find(a->param1);
	if(i != main_character->mappedAnimationIds.end()) {
		main_character->mappedAnimationIds.erase(i);
		map<int, CharacterAnimationSet*>::iterator ci = 
			main_character->animationSet.find(main_character->currentAnimationSetId);
		CharacterAnimationSet *set = ci->second;
		set->mappedAnimationIds = main_character->mappedAnimationIds;
	}
	SetCurrentCharacterAnimation(main_character, true);
	return 0;
}

int setGraphLineState(int sceneId, int lineId, bool state) {
	dout << " scene id: " << sceneId << ", line id: " << lineId << endl;
	SCMI scmi = all_scenes.find(sceneId);
	if(scmi == all_scenes.end()) {
		dout << "ERROR- could not find scene with such id" << endl;
		exit(1);
	}
	Scene *s = scmi->second;

	// FixMe:
	s->walkMask->setGraphLineState(lineId, state);
	return 0;
}

int enable_graph_line(ScriptAction *a) {
	dout << "  enable graph line" << endl;
	setGraphLineState(a->param1, a->param2, true);
	return 0;
}
int disable_graph_line(ScriptAction *a) {
	dout << "  disable graph line" << endl;
	setGraphLineState(a->param1, a->param2, false);
	return 0;
}

int trigger_interrupt_script(ScriptAction *a) {
	ScriptTrigger t;
	t.scriptId = a->param1;
	t.triggeredTill = current_time + a->param2;
	t.triggerType = SCRIPT_TRIGGER_INTERRUPT;

	dout << "  trigger interrupt script with id: " << a->param1
		<< ", will trigger in: " << t.triggeredTill << endl;

	triggeredScripts.insert(make_pair<int, ScriptTrigger>(t.triggeredTill, t));
	return 0;
}

int trigger_resume_script(ScriptAction *a) {
	ScriptTrigger t;
	t.scriptId = a->param1;
	t.triggeredTill = current_time + a->param2;
	t.triggerType = SCRIPT_TRIGGER_RESUME;

	dout << "  trigger resume script with id: " << a->param1
		<< ", will trigger in: " << t.triggeredTill << endl;

	triggeredScripts.insert(make_pair<int, ScriptTrigger>(t.triggeredTill, t));
	return 0;
}

int untrigger_script(ScriptAction *a) {
	dout << "  untrigger script: " << a->param1 << endl;
	map<int, ScriptTrigger>::iterator i = triggeredScripts.begin();
	while(i != triggeredScripts.end()) {
		ScriptTrigger &t = i->second;
		if(t.scriptId == a->param1) {
			triggeredScripts.erase(i);
			dout << "  untriggered" << endl;
			return 0;
		}
	}
	if(i == triggeredScripts.end()) {
		dout << "  script has not been triggered." << endl;
	}
	return 0;
}

int untrigger_all_scripts(ScriptAction *a) {
	dout << "  untrigger ALL triggered scripts" << endl;
	triggeredScripts.clear();
	return 0;
}

int change_dialog_question(ScriptAction *a) {
	dout << "  change dialog question" << endl;
	dout << "   dialog: " << a->param1 << ", question: " << a->param2 << endl;
	DMI dmi = all_dialogs.find(a->param1);
	if(dmi == all_dialogs.end()) {
		dout << "ERROR- could not find dialog with id: " << a->param1 << endl;
		exit(1);
	}
	Dialog *d = dmi->second;
	map<int, DialogQuestion>::iterator i = d->questions.find(a->param2);
	if(i == d->questions.end()) {
		dout << "ERROR- could not find dialog question with id: " << a->param2 << endl;
		exit(1);
	}
	DialogQuestion &q = i->second;
	q.questions[q.currentQuestionIdx] = a->str;
	dout << "   set dialog question to: '" << q.questions[q.currentQuestionIdx] << "'" << endl;
	return 0;
}
int change_dialog_answer(ScriptAction *a) {
	dout << "  change dialog answer" << endl;
	dout << "   dialog: " << a->param1 << ", answer: " << a->param2 << endl;
	DMI dmi = all_dialogs.find(a->param1);
	if(dmi == all_dialogs.end()) {
		dout << "ERROR- could not find dialog with id: " << a->param1 << endl;
		exit(1);
	}
	Dialog *d = dmi->second;
	map<int, DialogQuestion>::iterator qi = d->questions.begin();
	int changed = 0;
	while(qi != d->questions.end()) {
		DialogQuestion &q = qi->second;
		for(int c = 0; c < q.answers.size(); c++) {
			DialogAnswer &answ = q.answers[c];
			if(answ.id == a->param2) {

				string choose, real;
				if(!parseSayString(a->str, choose, real)) {
					real = choose;
				}

				answ.answerChoose[answ.currentAnswerIdx] = choose;
				answ.answerReal[answ.currentAnswerIdx] = real;

				dout << "   changed answer for question: " << q.id << " to: '" << a->str << "'" <<endl;
				changed++;
			}
		}
		qi++;
	}
	if(!changed) {
		dout << "ERROR- Invalid answer id specified" << endl;
		exit(1);
	}
	return 0;
}

int play_video(ScriptAction *a) {
	dout << "  play video: " << a->str << endl;
	stopTime();
	bool wasMusicOn = musicPlayer.isPlaying();
	playVideo(a->str);
//	if (wasMusicOn) {
//		musicPlayer.playNext();
//	}
	resumeTime();
	return 0;
}

int do_fade(ScriptAction *a) {
	dout << "  do fade effect: " << a->param1 << endl;

	glFlush();

	SDL_GL_SwapBuffers();

	
	shouldFade = true;
	fadeMethod = a->param1;
	fadeSpeed = a->param2;
	fadeProgress = -1;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, savedScreenTexture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, savedScreenTextureW, savedScreenTextureH, 0);
	dout << "save texture: " << savedScreenTextureW  << ", " << savedScreenTextureH << endl;

	//myBlitSurface(screen, NULL, savedScreen, NULL);
	
    SDL_GL_SwapBuffers();

	return 0;
}

int crossfade_music(ScriptAction *a) {
	dout << "  crossfade music: " << a->param1 << ", " << a->param2 << endl;
	musicPlayer.doSpecialEffect(MUSIC_CROSSFADE_SONGS, a->param1, a->param2);
	return 0;
}

int TOTAL_FADE_EFFECTS = 8;

int do_fade_random(ScriptAction *a) {
	dout << "  do random fade effect" << endl;
	int effect = current_time % TOTAL_FADE_EFFECTS;
	int origParam1 = a->param1;
	a->param1 = effect;
	a->param2 = origParam1;
	do_fade(a);
	a->param1 = origParam1;
	return 0;
}

int show_game_credits(ScriptAction *a) {
	dout << "  show game credits" << endl;
	showGameCredits();
	return 0;
}
int end_current_game(ScriptAction *a) {
	dout << "  end current game, end mode: " << a->param1 << endl;
	SDL_Event e;
	e.type = GAME_EVENT_END_CURRENT_GAME;
	e.user.code = a->param1;
	SDL_PushEvent(&e);
	return 0;
}

int wait_hotspot_anim_end(ScriptAction *a) {
	dout << "  wait hotspot: " << a->param1 << " animation end" << endl;
	return suspendScript(a);
}

int set_hotspot_depth(ScriptAction *a) {
	dout << "  set hotspot depth" << endl;
	dout << "    hotspot id: " << a->param1;
	dout << "    depth: " << a->param2;

	HMI hmi = all_hotspots.find(a->param1);
	if(hmi == all_hotspots.end()) {
		dout << "ERROR- unknown hotspot with id: " << a->param1 << endl;
		exit(1);
	}

	Hotspot *h = hmi->second;
	dout << "    hotspot current depth: " << h->depth << endl;
	if(h->depth == a->param2) {
		dout << "    same depth, nothing changed" << endl;
		return 0;
	}

	h->depth = a->param2;
	if(current_scene) {
		HMI sceneHMI =  current_scene->hotspots.find(h->id);
		if(sceneHMI != current_scene->hotspots.end()) {
			// Hotspot is in current scene
			dout << "    hotspot in current scene" << endl;
			if(h->enabled) {
				dout << "    hotspot is visible, resorting scene hotspots" << endl;
				ResortHotspots(current_scene);
			} else {
				dout << "    hotspot is not visible, no hotspot resorting required" << endl;
			}
		} else {
			dout << "    hotspot not in current_scene" << endl;
		}
	} else {
		dout << "    no current scene loaded" << endl;
	}
	return 0;
}

int wait_hotspot_anim_transform_end(ScriptAction *a) {
	dout << "  wait hotspot: " << a->param1 << " animation transformation end" << endl;
	return suspendScript(a);
}

int skip_actions_if(ScriptAction *a) {
	dout << "  skip " << a->param2 << "action(s) if '" << a->str << "' = " << a->param1 << endl;
	dout << "   " << a->str << " : " << game_states.get_value(a->str) << endl;
	if(game_states.get_value(a->str) == a->param1) {
		return a->param2;
	}
	return 0;
}

int char_say_text(ScriptAction *a) {
	dout << "  say char text: " << a->str << endl;

	DMI dmi = all_dialogs.find((unsigned) -2);
	if(dmi == all_dialogs.end()) {
		dout << "ERROR- Could not find predefined dialog with id -2" << endl;
		exit(1);
	}

	string answer;
	string soundFile;
	parseSayString(a->str, answer, soundFile);

	Dialog *d = dmi->second;
	DialogQuestion &q = d->questions.begin()->second;
	q.answers[0].answerReal[0] = answer;
	q.answers[0].soundFiles[0] = soundFile;

	ScriptAction tmp;
	tmp.action = ACTION_START_DIALOGUE;
	tmp.param1 = -2;
	tmp.param2 = -1;

	return start_dialogue(&tmp);
}

int npc_say_text(ScriptAction *a) {
	dout << "  say npc text: " << a->str << endl;

	DMI dmi = all_dialogs.find((unsigned) -3);
	if(dmi == all_dialogs.end()) {
		dout << "ERROR- Could not find predefined dialog with id -3" << endl;
		exit(1);
	}
	
	string question;
	string soundFile;
	parseSayString(a->str, question, soundFile);
	
	Dialog *d = dmi->second;
	DialogQuestion &q = d->questions.begin()->second;
	q.questions[0] = question;
	q.soundFiles[0] = soundFile;

	ScriptAction tmp;
	tmp.action = ACTION_START_DIALOGUE;
	tmp.param1 = -3;
	tmp.param2 = -1;
	return start_dialogue(&tmp);
	
}

int set_script_execution_count(ScriptAction *a) {
	dout << "  set script " << a->param1 << "execution count to " << a->param2 << endl;
	SMI i = all_scripts.find(a->param1);
	if(i == all_scripts.end()) {
		dout << "ERROR- Unknown script with id: " << a->param1 << endl;
		exit(1);
	}
	Script *s = i->second;
	s->execute_times = a->param2;
	return 0;
}
