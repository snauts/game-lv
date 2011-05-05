#include "game.h"
#include "consts.h"

int ScriptAction::tmpval[SCRIPT_ACTION_STATIC_DATA_SIZE] = {0,0,0,0,0};

unsigned suspendedScriptActionCode[SUSPENDABLE_ACTIONS] = {  
	ACTION_START_DIALOGUE,
	ACTION_MOVE_CHARACTER,
	ACTION_PLAY_CHARACTER_ANIMATION,
	ACTION_WAIT_TIME,
	ACTION_PLAY_SOUND,
	ACTION_PLAY_HOTSPOT_ANIMATION,
	ACTION_WAIT_HOTSPOT_ANIM_END,
	ACTION_WAIT_HOTSPOT_ANIM_TRANSFORM_END
};

int isDialogComplete();
int isCharacterMoved();
int isCharAnimPlayed();
int isTimePassed();
int isSoundPlayed();
int isHotspotAnimPlayed();
int isHotspotAnimEnded();
int isHotspotAnimTransformEnded();

isActionCompleteHandler suspendedScriptActionHandlers[SUSPENDABLE_ACTIONS] = { 
	isDialogComplete,
	isCharacterMoved,
	isCharAnimPlayed,
	isTimePassed,
	isSoundPlayed,
	isHotspotAnimPlayed,
	isHotspotAnimEnded,
	isHotspotAnimTransformEnded
};

static int lastScriptAutonumberId = 1000000000;

int suspendScript(ScriptAction *a) {
	// Returns:
	// <0 : Error
	// 0 : Script should be continued
	// >0 : Ok, script suspended
	
	int ret = 0;

	cancelSuspendedScript();
	switch(a->action) {
	case ACTION_START_DIALOGUE:
		{
			DMI i = all_dialogs.find(a->param1);
			if(i == all_dialogs.end()) {
				dout << "ERROR- There is no dialogue with id: " << a->param1 << endl;
				exit(1);
//				break;
			}
			Dialog *d = i->second;
			startDialog(d, a->param2);
			ret = 1;
			break;
		}
	case ACTION_MOVE_CHARACTER:
		{
			ret = startCharacterMovement(a->param1, a->param2);
			if(ret == 0) {
				// We already are on spot, but thanks to the "walk-to-the-closest-spot"
				// feature we should check whether this is point where we want to go.
				if((main_character->coord.x != a->param1) || (main_character->coord.y != a->param2)) {
					// We are not on spot where we should have been, so return error.
					ret = -1;
				}
			}
			break;
		}
	case ACTION_PLAY_CHARACTER_ANIMATION:
		{
			map<int, CharacterAnimation*>::iterator i = main_character->anims.find(a->param1);
			if(i == main_character->anims.end()) {
				dout << "WARNING: Could not find character animation: " << a->param1 << endl;
				break;
			}
			CharacterAnimation *anim = i->second;
			if(anim->anims.size() < 1) {
				dout << "WARNING: Character animation: " << a->param1 << " has 0 directions" << endl;
				break;
			}

			double rad_per_anim = ((2.0 * PI_COEF) / (double)anim->directions);
			double new_dir = main_character->coord.angle + (rad_per_anim / 2.0);
			if(new_dir < 0.0) {
				new_dir += PI_COEF * 2.0;
			} else if(new_dir >= (2.0 * PI_COEF)) {
				new_dir -= PI_COEF * 2.0;
			}
			int current_anim = (int)(new_dir / rad_per_anim) % anim->directions;
			
			for(unsigned tmpVal = 0; tmpVal < anim->animationsPerDirection; tmpVal++) {
				resetAnimation(anim->anims[current_anim]->anim[tmpVal]);
			}

			main_character->current_anim = anim->anims[current_anim]->anim;
			main_character->currentAnimSize = anim->animationsPerDirection;
			main_character->current_speed = anim->moveSpeed;

			// Little hack...
			a->tmpval[0] = anim->anims[current_anim]->anim[anim->animationsPerDirection - 1]->currentLoops;
			
			setCharacterState(CHARACTER_ANIM);
			soundPlayer.playCharacterSound(anim->sound);

			ret = 1;
			break;
		}
	case ACTION_WAIT_TIME:
		{
			ret = 1;
			a->tmpval[0] = current_time;
			break;
		}
	case ACTION_PLAY_SOUND:
		{
			// Sound is already started in script action.
			// so we just suspend script.
			ret = 1;
			break;
		}
	case ACTION_PLAY_HOTSPOT_ANIMATION:
		{
			HMI hotspotIter = all_hotspots.find(a->param1);
			if(hotspotIter != all_hotspots.end()) {
				Hotspot *h = hotspotIter->second;
				a->tmpval[0] = h->anim_id; // store old anim id
				if(!SetHotspotAnim(h, a->param2)) {
					// all ok anim found and set.
					a->tmpval[1] = h->anim->currentLoops; // remember current loop count.
					ret = 1;
				}
				soundPlayer.playHotspotSound(h->id, h->sound);
			}
			break;
		}
	case ACTION_WAIT_HOTSPOT_ANIM_END:
		{
			HMI hotspotIter = all_hotspots.find(a->param1);
			if(hotspotIter != all_hotspots.end()) {
				Hotspot *h = hotspotIter->second;
				// If hotspot is not visible there is no need to wait
				if(!h->enabled) {
					break;
				}
				if(a->param2 == 1) {
					if((h->anim->currentBlockIdx == 0) && (h->anim->currentFrame == 0)) {
						break;
					}
				}
				a->tmpval[0] = h->anim_id; // store old anim id
				a->tmpval[1] = h->anim->currentLoops; // remember current loop count.
				ret = 1;
			}
			break;
		}
	case ACTION_WAIT_HOTSPOT_ANIM_TRANSFORM_END:
		{
			HMI hotspotIter = all_hotspots.find(a->param1);
			if(hotspotIter != all_hotspots.end()) {
				Hotspot *h = hotspotIter->second;
				// If hotspot is not visible there is no need to wait
				if(!h->enabled) {
					break;
				}
				a->tmpval[0] = h->anim_id; // store old anim id
				a->tmpval[1] = h->anim->currentTransformLoops; // remember current loop count.
				ret = 1;
			}
			break;
		}
	default:
		{
			dout << "ERROR: Unknown action type needs to be suspended: " << a->action << endl;
			exit(1);
			return -1;
		}
	}

	// Install action complete handler.
	if(ret > 0) {
		for(int tmp = 0; tmp < SUSPENDABLE_ACTIONS; tmp++) {
			if(suspendedScriptActionCode[tmp] == a->action) {
				main_character->isActionComplete = suspendedScriptActionHandlers[tmp];
				break;
			}
		}
	} else {
		main_character->isActionComplete = NULL;
	}
	dout << " Ok, script suspended" << endl;
	return ret;
}
int cancelSuspendedScript() {
	if(main_character->suspended_script) {
		switch(main_character->suspended_action->action) {
		case ACTION_MOVE_CHARACTER:
			{
				stopCharacterMovement();
				break;
			}
		case ACTION_PLAY_CHARACTER_ANIMATION:
			{
				// Possible Todo.
				// Normally this should be true.
				if(main_character->state == CHARACTER_ANIM)	
					stopCharacterMovement();
				break;
			}
		case ACTION_START_DIALOGUE:
		case ACTION_CHAR_SAY_TEXT:
		case ACTION_NPC_SAY_TEXT:
			{
				// In dialog is executed script which wants to suspend.
				// So original script which started dialog, won't continue.
				break;
			}
		case ACTION_WAIT_TIME:
			{
				break;
			}
		case ACTION_PLAY_SOUND:
			{
				soundPlayer.stopScriptSound(main_character->suspended_action->param1);
				break;
			}
		case ACTION_PLAY_HOTSPOT_ANIMATION:
			{
				HMI hotspotIter = all_hotspots.find(main_character->suspended_action->param1);
				if(hotspotIter == all_hotspots.end()) {
					dout << "ERROR- in cancelSuspendedScript::ACTION_PLAY_HOTSPOT_ANIMATION, Could not find hotspot: " << main_character->suspended_action->param1 << endl;
					exit(1);
				}
				Hotspot *h = hotspotIter->second;
				SetHotspotAnim(h, main_character->suspended_action->tmpval[0]);
				break;
			}
		case ACTION_WAIT_HOTSPOT_ANIM_END:
			{
				break;
			}
		case ACTION_WAIT_HOTSPOT_ANIM_TRANSFORM_END:
			{
				break;
			}
		default:
			{
				dout << "ERROR- Canceling unknown script action: " << main_character->suspended_action->action << endl;
				exit(1);
			}
		}
	}
	main_character->suspended_script = NULL;
	main_character->suspended_action = NULL;
	main_character->isActionComplete = NULL;
	memset(ScriptAction::tmpval, 0, sizeof(ScriptAction::tmpval));
	return 0;
}

int completeSuspendedScript() {
	Script* s = main_character->suspended_script;
	ScriptAction* a = main_character->suspended_action;
	cancelSuspendedScript();
	resumeScript(s, a, true);
	return 0;
}

int isDialogComplete() {
	if(!currentDialog)
		return 1;
	else
		return 0;
}
int isCharacterMoved() {

	if(main_character->state == CHARACTER_STAND) {
		if((main_character->coord.x == main_character->suspended_action->param1) &&
			(main_character->coord.y == main_character->suspended_action->param2)) {
			// All ok, we are in the right place.
			return 1;
		} else {
			// Error- we are standing, but not in the right place.
			dout << "WARNING: Could not go to the selected point: " << main_character->suspended_action->param1 
				<< "," << main_character->suspended_action->param2 << " script " 
				<< main_character->suspended_script->id << " won't continue." << endl;
			cancelSuspendedScript();
			return 0;
		}
	}
	return 0;
}
int isCharAnimPlayed() {

	if(main_character->current_anim[main_character->currentAnimSize - 1]->currentLoops 
		!= main_character->suspended_action->tmpval[0]) {
		if((main_character->prevState == CHARACTER_WALK) && (main_character->suspended_action->param2)) {
			setCharacterState(CHARACTER_WALK);
			SetCurrentCharacterAnimation(main_character);
			current_scene->walkMask->updateTime(current_time);
		} else {
			stopCharacterMovement();
		}
		dout << "character animation ended in: " << current_time << endl;
		return 1;
	}
	return 0;
}

int isTimePassed() {
	if(main_character->suspended_action->param1 + main_character->suspended_action->tmpval[0] < current_time) {
		return 1;
	}
	return 0;
}

int isSoundPlayed() {
	return (!soundPlayer.isPlayingScriptSound(main_character->suspended_action->param1));
}

int isHotspotAnimPlayed() {
	HMI hotspotIter = all_hotspots.find(main_character->suspended_action->param1);
	if(hotspotIter == all_hotspots.end()) {
		dout << "ERROR- in isHotspotAnimPlayed::ACTION_PLAY_HOTSPOT_ANIMATION, Could not find hotspot: " << main_character->suspended_action->param1 << endl;
		exit(1);
	}
	Hotspot *h = hotspotIter->second;
	if(h->anim->currentLoops > main_character->suspended_action->tmpval[1]) {
		return 1;
	}
	return 0;
}

int isHotspotAnimEnded() {
	HMI hotspotIter = all_hotspots.find(main_character->suspended_action->param1);
	if(hotspotIter == all_hotspots.end()) {
		dout << "ERROR- in isHotspotAnimEnded::ACTION_PLAY_HOTSPOT_ANIMATION, Could not find hotspot: " << main_character->suspended_action->param1 << endl;
		exit(1);
	}
	Hotspot *h = hotspotIter->second;
	if(h->anim->currentLoops > main_character->suspended_action->tmpval[1]) {
		return 1;
	}
	return 0;
}

int isHotspotAnimTransformEnded() {
	HMI hotspotIter = all_hotspots.find(main_character->suspended_action->param1);
	if(hotspotIter == all_hotspots.end()) {
		dout << "ERROR- in isHotspotAnimTransformEnded::ACTION_PLAY_HOTSPOT_ANIMATION, Could not find hotspot: " << main_character->suspended_action->param1 << endl;
		exit(1);
	}
	Hotspot *h = hotspotIter->second;
	if(h->anim->currentTransformLoops > main_character->suspended_action->tmpval[1]) {
		return 1;
	}
	return 0;
}

void LoadAllScripts(ScriptMap& m) {
	dout << "--- LoadAllScript" << endl;
	m.clear();
	
	vector<string> fileNames;
	loadListFile("scripts.lst", fileNames);
	
	FILE* fin;
	char str[256];
	Script* s;
	for(int currFile = 0; currFile < fileNames.size(); currFile++) {
		strcpy(str, fileNames[currFile].c_str());
		fin = fopen(str, "r");
		if(!fin) {
			dout << "ERROR- 'scripts.lst' refers to invalid file: '" << str << "'" << endl;
			exit(1);
		}
		dout << " Processing file: " << str << endl;
		while((s = LoadScript(fin))) {
			if(s->id == -1) { // Assign automatical id to the script 
				s->id = lastScriptAutonumberId;
				lastScriptAutonumberId++;
				dout << "  automatical id assigned: " << s->id << endl;
			}
			if(m.find(s->id) != m.end()) {
				dout << "ERROR- script with id: " << s->id << " already exists. File: " << str << endl;
				exit(1);
			}
			m.insert(make_pair<unsigned, Script*>(s->id, s));
			dout << " Script with id: " << s->id << " loaded" << endl;
		}
		fclose(fin);
	}
	dout << "--- End LoadAllScript" << endl << endl;
}
Script* LoadScript(FILE* fin) {
	if(!IsNextString(fin, "script")) {
		dout << " Loading script failed- no 'script' tag found" << endl;
		return NULL;
	}
	int id = LoadInt(fin);
	int execute_count = LoadInt(fin);
	int execute_on = LoadInt(fin);
	int param1 = 0, param2 = 0;
	Script* scr;
	switch(execute_on) {
	case SCRIPT_USE_ITEM:
	case SCRIPT_ENTER_SCENE:
	case SCRIPT_COMBINE_ITEM:
		{
			param1 = LoadInt(fin);
			param2 = LoadInt(fin);
			break;
		}
	default:
		{
			break;
		}
	}
	scr = new Script;
	scr->execute_times = execute_count;
	scr->id = id;
	scr->param1 = param1;
	scr->param2 = param2;
	scr->type = execute_on;
	unsigned action_type = 0;
	ScriptAction* action;
	string action_str;
	while(IsNextString(fin, "action")) {

		action_type = LoadInt(fin);

		param1 = 0;
		param2 = 0;
		action_str = "";

		switch(action_type) {
		case ACTION_DO_NOTHING:
		case ACTION_ENABLE_USER_INPUT:
		case ACTION_DISABLE_USER_INPUT:
		case ACTION_STOP_MUSIC:
		case ACTION_PAUSE_MUSIC:
		case ACTION_RESUME_MUSIC:
		case ACTION_SHOW_CHARACTER:
		case ACTION_HIDE_CHARACTER:
		case ACTION_UNTRIGGER_ALL_SCRIPTS:
		case ACTION_SHOW_GAME_CREDITS:
			{
				break;
			}

		case ACTION_ADD_INVENTORY_ITEM:
		case ACTION_REMOVE_INVENTORY_ITEM:
		case ACTION_SHOW_HOTSPOT:
		case ACTION_HIDE_HOTSPOT:
		case ACTION_SET_CHARACTER_DIRECTION:
		case ACTION_WAIT_TIME:
		case ACTION_STOP_SOUND:
		case ACTION_SET_CHARACTER_ANIMATION_SET:
		case ACTION_UNMAP_CHARACTER_ANIMATION:
		case ACTION_UNTRIGGER_SCRIPT:
		case ACTION_DO_FADE_RANDOM:
		case ACTION_END_CURRENT_GAME:
		case ACTION_WAIT_HOTSPOT_ANIM_TRANSFORM_END:
			{
				param1 = LoadInt(fin);
				break;
			}

		case ACTION_CHANGE_HOTSPOT_ANIMATION:
		case ACTION_MOVE_CHARACTER:
		case ACTION_SET_CHARACTER_COORDINATES:
		case ACTION_PLAY_CHARACTER_ANIMATION:
		case ACTION_ENABLE_DIALOG_ANSWER:
		case ACTION_DISABLE_DIALOG_ANSWER:
		case ACTION_PLAY_MUSIC:
		case ACTION_START_SOUND:
		case ACTION_PLAY_SOUND:
		case ACTION_PLAY_HOTSPOT_ANIMATION:
		case ACTION_MAP_CHARACTER_ANIMATION:
		case ACTION_ENABLE_GRAPH_LINE:
		case ACTION_DISABLE_GRAPH_LINE:
		case ACTION_TRIGGER_INTERRUPT_SCRIPT:
		case ACTION_TRIGGER_RESUME_SCRIPT:
		case ACTION_DO_FADE:
		case ACTION_CROSSFADE_MUSIC:
		case ACTION_WAIT_HOTSPOT_ANIM_END:
		case ACTION_SET_HOTSPOT_DEPTH:
		case ACTION_SET_SCRIPT_EXECUTION_COUNT:
			{
				param1 = LoadInt(fin);
				param2 = LoadInt(fin);
				break;
			}

		case ACTION_EXECUTE_SCRIPT_IF:
		case ACTION_SKIP_ACTIONS_IF:
			{
				param1 = LoadInt(fin);
				param2 = LoadInt(fin);
				action_str = LoadString(fin);
				break;
			}
		case ACTION_DISPLAY_TEXT_STRING:
		case ACTION_CHANGE_DIALOG_QUESTION:
		case ACTION_CHANGE_DIALOG_ANSWER:
			{
				param1 = LoadInt(fin);
				param2 = LoadInt(fin);
				action_str = LoadString(fin, true);
				break;
			}

		case ACTION_SET_STATE:
		case ACTION_STOP_SCRIPT_IF:
		case ACTION_STOP_SCRIPT_IFNOT:
			{
				param1 = LoadInt(fin);
				action_str = LoadString(fin);
				break;
			}

		case ACTION_PLAY_VIDEO:
			{
				action_str = LoadString(fin);
				break;
			}
		case ACTION_CHAR_SAY_TEXT:
		case ACTION_NPC_SAY_TEXT:
			{
				action_str = LoadString(fin, true);
				break;
			}
		case ACTION_START_NEW_SCENE:
		case ACTION_START_DIALOGUE:
			{
				param1 = LoadInt(fin);
				if(!IsNextString(fin, "end_action", false)) {
					param2 = LoadInt(fin);
				} else {
					param2 = -1;
				}
				break;
			}

		default:
			{
				dout << "ERROR- LoadScript failed- unknown action: " << action_type << endl;
				exit(1);
			}

		}
		if(!IsNextString(fin, "end_action")) {
			dout << "ERROR- Loading script failed- no 'end_action' tag found" << endl;
			exit(1);
		}
		action = new ScriptAction;
		action->action = action_type;
		action->param1 = param1;
		action->param2 = param2;
		action->str = action_str;
		scr->actions.push_back(action);
	}
	if(!IsNextString(fin, "end_script")) {
		dout << "ERROR- Loading script failed- no 'end_script' tag found" << endl;
		exit(1);
	}
	return scr;
}


unsigned executeSceneStartupScripts(Scene* curr_scene, Scene* next_scene) {
	dout << "--- ExecuteSceneStartupScripts" << endl;
	int prev_id;
	int next_id = next_scene->id;
	if(!curr_scene)
		prev_id = -1;
	else
		prev_id = curr_scene->id;

	SMI i = all_scripts.begin();
	Script* tmp;
	while(i != all_scripts.end()) {
		tmp = i->second;
		if(tmp->type == SCRIPT_ENTER_SCENE) {
			if(tmp->param2 == next_id) {
				if((prev_id == tmp->param1) || (tmp->param1 == -1)) {
					executeScript(tmp);
				}
			}
		}
		i++;
	}
	dout << "--- EndExecuteSceneStartupScripts" << endl;
	return 0;
}

unsigned doStartupScript() {
	dout << "--- DoStartupScript" << endl;
	SMI i = all_scripts.begin();
	Script* s;
	while(i != all_scripts.end()) {
		s = i->second;
		if(s->type == SCRIPT_STARTUP) {
			executeScript(s);
		}
		i++;
	}
	dout << "--- End DoStartupScript" << endl << endl;
	return 0;
}

int executeScriptById(int scriptId) {
	if(scriptId == -1) {
		return 0;
	}

	SMI si = all_scripts.find(scriptId);
	if(si != all_scripts.end()) {
		Script *s = si->second;
		return executeScript(s);
	} else {
		dout << "ERROR- Could not execute script with id: " << scriptId << ", no such script!!!" << endl;
		exit(1);
	}
	return 0;
}

