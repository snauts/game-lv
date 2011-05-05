#include "game.h"
#include "structs.h"

int readAnimationState(FILE *fin, Animation *anim);
int readAnimationTransformState(FILE *fin, Animation *anim);
int readScript(FILE *fin, Script *script);
int readStateMachine(FILE *fin, state_machine<string, int, debug_out> &m);

int readMainCharacter(string filePrefix);
int readAllScripts(string filePrefix);
int readGameState(string filePrefix);
int readStates(string filePrefix);
int readHotspots(string filePrefix);
int readScenes(string filePrefix);
int readInventory(string filePrefix);
int readDialogs(string filePrefix);
int readMusic(string filePrefix);

int loadGame(string filePrefix) {
	dout << "--- loadGame (started in: " << current_time << ")" << endl;
	dout << " loading game from '" << filePrefix << "'" << endl;
	resetGame();
	if(readStates(filePrefix)) return 1;
	if(readAllScripts(filePrefix)) return 1;
	if(readDialogs(filePrefix)) return 1;
	if(readGameState(filePrefix)) return 1;
	if(readInventory(filePrefix)) return 1;
	if(readHotspots(filePrefix)) return 1;
	if(readScenes(filePrefix)) return 1;
	if(readMainCharacter(filePrefix)) return 1;
	if(readMusic(filePrefix)) return 1;
	soundPlayer.resumeAll();

	dout << "--- End loadGame (ended in: " << current_time << ")" << endl << endl;
	return 0;
}

int readGameState(string filePrefix) {
	dout << " loadGameState" << endl;	

	string fileName = filePrefix + "game.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadGameState: Could not open " << fileName << " for reading." << endl;
		return 1;
	}

	current_time = readInt(fin);
	current_game_state = readInt(fin);
	state_var1 = readInt(fin);
	state_var2 = readInt(fin);

	dout << "  time: " << current_time << endl;
	dout << "  state: " << current_game_state << endl;
	dout << "  state_var1: " << state_var1 << endl;
	dout << "  state_var2: " << state_var2 << endl;

	int currentDialogId = readInt(fin);
	dout << "  current dialog id: " << currentDialogId << endl;
	if(currentDialogId != -1) {
		int currentQuestionId = readInt(fin);
		int currentAnswerId = readInt(fin);
		dout << "  current dialog answer id: " << currentAnswerId << endl;
		dout << "  current dialog question id: " << currentQuestionId << endl;

		DMI dmi = all_dialogs.find(currentDialogId);
		if(dmi == all_dialogs.end()) {
			dout << "ERROR- Dialog: " << currentDialogId << " does not exist" << endl;
			return 1;
		}
		currentDialog = dmi->second;
		
		map<int, DialogQuestion>::iterator qi = currentDialog->questions.find(currentQuestionId);
		if(qi == currentDialog->questions.end()) {
			dout << "ERROR- Invalid question id: " << currentQuestionId << endl;
			return 1;
		}
		currentDialogQuestion = &(qi->second);

		if(currentAnswerId >= 0) {
			if(currentAnswerId >= currentDialogQuestion->answers.size()) {
				dout << "ERROR- Invalid answer id: " << currentAnswerId << endl;
				return 1;
			}
			currentDialogAnswer = &(currentDialogQuestion->answers[currentAnswerId]);
		}

		if((state_var1 == DIALOG_SHOW_QUESTION) || (state_var1 == DIALOG_SHOW_ANSWER)){
			int isSound = readInt(fin);
			if(isSound) {
				string soundFile;
				if(state_var1 == DIALOG_SHOW_QUESTION) {
					 soundFile = currentDialogQuestion->soundFiles[currentDialogQuestion->currentQuestionIdx];
				} else {
					soundFile = currentDialogAnswer->soundFiles[currentDialogAnswer->currentAnswerIdx];
				}
				soundPlayer.loadDialogSoundState(fin, soundFile);
			}
		}
		
	}

	currentViewX = readInt(fin);
	currentViewY = readInt(fin);

	// Save current visible text strings.
	int totalText = readInt(fin);
	should_draw_text = (totalText > 0);
	for(int a = 0; a < totalText; a++) {
		TextItem i;
		i.text = readString(fin);
		i.expireTime = readInt(fin);
		i.fontId = readInt(fin);
		FMI fmi = allFonts.find(i.fontId);
		if(fmi == allFonts.end()) {
			dout << "ERROR- loading text item: '" << i.text 
				<< "'. Font with id: " << i.fontId << " does not exist." << endl;
			exit(1);
		}
		i.font = fmi->second;
		onscreen_text.push_back(i);
	}

	fclose(fin);
	dout << " End loadGameState" << endl;
	return 0;
}

int readAnimationState(FILE *fin, Animation *anim) {
	anim->currentBlockIdx = readInt(fin);
	anim->currentFrame = readInt(fin);
	anim->currentLoops = readInt(fin);
	anim->lastUpdate = readInt(fin);
	int totalBlocks = readInt(fin);
	if(( totalBlocks != anim->totalBlocks) || (anim->currentBlockIdx >= anim->totalBlocks)) {
		dout << "ERROR- animation out of sync or is modified." << endl;
		exit(1);
	}
	
	anim->useVideoMemory = (readInt(fin) > 0);
	anim->isImageDataLoaded = (readInt(fin) > 0);
	if(anim->isImageDataLoaded) {
		loadAnimationData(anim);
	}

	anim->currentBlock = anim->blocks[anim->currentBlockIdx];
	dout << "LoadAnimState currentFrame: " << anim->currentFrame 
		<< ", last update: " << anim->lastUpdate
		<< ", current time: " << current_time << endl;
	readAnimationTransformState(fin, anim);
	return 0;
}

int readScript(FILE *fin, Script *script) {
	script->id = readInt(fin);
	script->execute_times = readInt(fin);
	script->type =  readInt(fin);
	script->param1 = readInt(fin);
	script->param2 = readInt(fin);

	int totalActions = readInt(fin);
	for(int a = 0; a < totalActions; a++) {
		ScriptAction *action = new ScriptAction;
		action->action = readInt(fin);
		action->param1 = readInt(fin);
		action->param2 = readInt(fin);
		action->param3 = readInt(fin);
		action->param4 = readInt(fin);
		action->str = readString(fin);
		
		script->actions.push_back(action);
	}

	return 0;
}

int readStateMachine(FILE *fin, state_machine<string, int, debug_out> &m) {
	int totalStates = readInt(fin);
	for(int a = 0; a < totalStates; a++) {
		string state = readString(fin);
		int value = readInt(fin);
		m.set_value(state, value);
	}
	return 0;
}

int readMainCharacter(string filePrefix) {
	dout << " loadMainCharacter" << endl;	
	
	string fileName = filePrefix + "character.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadMainCharacter: Could not open " << fileName << " for reading." << endl;
		return 1;
	}

	main_character = new Character;
	main_character->state = readInt(fin);
	main_character->prevState = readInt(fin);
	main_character->lastUpdate = current_time - readInt(fin);
	main_character->visible = (readInt(fin) > 0);
	main_character->currentAnimationSetId = -1;
	int currentAnimationSetId = readInt(fin);

	// Load all animation sets.

	int totalAnimSets = readInt(fin);
	int a,b;
	for(a = 0; a < totalAnimSets; a++) {
		CharacterAnimationSet *animSet = new CharacterAnimationSet;
		animSet->id = readInt(fin);
		animSet->characterName = readString(fin);
		
		int totalAnims = readInt(fin);
		for(b = 0; b < totalAnims; b++) {
			CharacterAnimation *charAnim = new CharacterAnimation;
			charAnim->id = readInt(fin);
			charAnim->moveSpeed = readInt(fin);
			charAnim->soundFile = readString(fin);
			charAnim->directions = readInt(fin);
			charAnim->animationsPerDirection = readInt(fin);

			for(int c = 0; c < charAnim->directions; c++) {
				CharOneWayAnim *oneWayAnim = new CharOneWayAnim;
				for(int d = 0; d < charAnim->animationsPerDirection; d++) {
					string gfxFile = readString(fin);
					oneWayAnim->gfxFile.push_back(gfxFile);
				}
				charAnim->anims.push_back(oneWayAnim);
			}
			animSet->anims.insert(make_pair<int, CharacterAnimation*>(charAnim->id, charAnim));
		}
		int totalMappedAnims = readInt(fin);
		for(b = 0; b < totalMappedAnims; b++) {
			int first = readInt(fin);
			int second = readInt(fin);
			animSet->mappedAnimationIds.insert(make_pair<int, int>(first, second));
		}
		main_character->animationSet.insert(make_pair<int, CharacterAnimationSet*>(animSet->id, animSet));
	}
	// All anim sets loaded.

	setCharacterAnimationSet(currentAnimationSetId);
	main_character->currentAnimationSetId = currentAnimationSetId;

	
	int currAnimId = readInt(fin);
	int currAnimDirection = readInt(fin);
	int currAnimSize = readInt(fin);

	// Heavy magic... 
	map<int, CharacterAnimation*>::iterator animIter = main_character->anims.find(currAnimId);
	CharacterAnimation *charAnim = animIter->second;
	CharOneWayAnim *oneWayAnim = charAnim->anims[currAnimDirection];
	main_character->current_anim = oneWayAnim->anim;
	main_character->currentAnimSize = currAnimSize;

	for(a = 0; a < currAnimSize; a++) {
		readAnimationState(fin, main_character->current_anim[a]);
	}

	if(charAnim->sound) {
		loadSoundCollectionState(fin, charAnim->sound);
		soundPlayer.loadCharacterSoundState(fin, charAnim->sound);
	}
	// End of heavy magic.


	// Load suspended scripts.
	int suspededScriptId = readInt(fin);
	if(suspededScriptId >= 0) {
		int suspendedActionNumber = readInt(fin);
		int suspendedActionHandler = readInt(fin);
		for(a = 0; a < SCRIPT_ACTION_STATIC_DATA_SIZE; a++) {
			ScriptAction::tmpval[a] = readInt(fin);
		}
		SMI smi = all_scripts.find(suspededScriptId);
		if(smi == all_scripts.end()) {
			dout << "ERROR- Unknown script suspeded: " << suspededScriptId << endl;
			exit(1);
		}
		main_character->suspended_script = smi->second;
		main_character->suspended_action = main_character->suspended_script->actions[suspendedActionNumber];
		main_character->isActionComplete = suspendedScriptActionHandlers[suspendedActionHandler];
	}

	CoordinatePoint::readCoordinatePoint(fin, main_character->coord);

	fclose(fin);
	dout << " End loadMainCharacter" << endl;	
	return 0;
}

int readAllScripts(string filePrefix) {
	dout << " loadAllScripts" << endl;	
	
	string fileName = filePrefix + "scripts.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadScripts: Could not open " << fileName << " for reading." << endl;
		return 1;
	}

	int totalScripts = readInt(fin);
	dout << "  Total scripts: " << totalScripts << endl;
	for(int a = 0; a < totalScripts; a++) {
		Script *s = new Script;
		readScript(fin, s);
		dout << "   loaded script: " << s->id << endl;
		all_scripts.insert(make_pair<int, Script*>(s->id, s));
	}
	// Load script sound
	int playingChannels = readInt(fin);
	int channel;
	int soundId;
	SoundCollection *sound;
	map<int, SoundCollection*>::iterator soundIter;
	dout << "   loading script sounds" << endl;
	for(int b = 0; b < playingChannels; b++) {
		channel = readInt(fin);
		soundId = readInt(fin);
		soundIter = scriptSounds.find(soundId);		
		if(soundIter == scriptSounds.end()) {
			dout << "ERROR- Script sound loading out of sync, id: " << soundId << endl;
			exit(1);
		}
		dout << "    loading script sound on channel: " 
			<< channel - SOUND_CHANNEL_SCRIPT << ", with id: " << soundId << endl;
		sound = soundIter->second;
		loadSoundCollectionState(fin, sound);
		soundPlayer.loadScriptSoundChannel(fin, channel, sound);
	}
	dout << "   loading script sound complete" << endl;

	fclose(fin);
	return 0;
}

int readStates(string filePrefix) {
	dout << " loadStates" << endl;	
	
	string fileName = filePrefix + "state_machine.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadStates: Could not open " << fileName << " for reading." << endl;
		return 1;
	}
	readStateMachine(fin, game_states);
	dout << game_states;
	fclose(fin);
	dout << " End loadStates" << endl;	
	return 0;
}

int readHotspots(string filePrefix) {
	dout << " loadHotspots" << endl;	
	
	string fileName = filePrefix + "hotspots.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadHotspots: Could not open " << fileName << " for reading." << endl;
		return 1;
	}

	int totalHotspots = readInt(fin);
	dout << "  total hotspots: " << totalHotspots << endl;

	int a,b;

	for(a = 0; a < totalHotspots; a++) {
		Hotspot *h = new Hotspot;
		h->id = readInt(fin);
		dout << "   hotspot: " << h->id << endl;
		h->anim_id = readInt(fin);
		h->depth = readInt(fin);
		h->enabled = (readInt(fin) > 0);

		int animTotal = readInt(fin);
		for(b = 0; b < animTotal; b++) {
			HotspotAnim *ha = new HotspotAnim;
			ha->id = readInt(fin);
			dout << "    hotspot anim: " << ha->id << endl;
			ha->x = readInt(fin);
			ha->y = readInt(fin);
			ha->restartSoundOnAnimLoop = (readInt(fin) > 0);
			ha->description = readString(fin);
			ha->gfxName = readString(fin);
			ha->maskFile = readString(fin);
			ha->soundFile = readString(fin);

			h->anims.insert(make_pair<int, HotspotAnim*>(ha->id, ha));
		}
		LoadHotspotData(h);
		readAnimationState(fin, h->anim);

		int totalScripts = readInt(fin);
		for(b = 0; b < totalScripts; b++) {
			int itemId = readInt(fin);
			int scriptId = readInt(fin);
			dout << "   item: " << itemId << ", script: " << scriptId << endl;
			h->scripts.insert(make_pair<int, int>(itemId, scriptId));
		}
		all_hotspots.insert(make_pair<int, Hotspot*>(h->id, h));		
	}

	int playingHotspots = readInt(fin);
	int hotspotId;
	int hasSound;
	for(a = 0; a < playingHotspots; a++) {
		hotspotId = readInt(fin);
		hasSound = readInt(fin);
		if(hasSound) {
			HMI hmi = all_hotspots.find(hotspotId);
			if(hmi == all_hotspots.end()) {
				dout << "ERROR- Could not find hotspot: " << hotspotId << endl;
				exit(1);
			}
			Hotspot *h = hmi->second;
			if(!h->sound) {
				dout << "ERROR- hotspot sound loading out of sync, id: " << hotspotId << endl;
				exit(1);
			}
			loadSoundCollectionState(fin, h->sound);
			soundPlayer.loadHotspotSoundState(fin, hotspotId, h->sound);
		}
	}

	fclose(fin);
	dout << " End loadHotspots" << endl;	

	return 0;
}

int readScenes(string filePrefix) {
	dout << " loadScenes" << endl;	
	
	string fileName = filePrefix + "scenes.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadScenes: Could not open " << fileName << " for reading." << endl;
		return 1;
	}

	int currentSceneId = readInt(fin);
	int totalScenes = readInt(fin);

	dout << "  total scenes: " << totalScenes << endl;
	for(int a = 0; a < totalScenes; a++) {
		Scene *s = new Scene;
		s->id = readInt(fin);
		dout << "   scene: " << s->id << endl;
		s->gfxFile = readString(fin);
		s->maskFile = readString(fin);
		s->soundFile = readString(fin);

		int hotspotsTotal = readInt(fin);		
		int hotspotId;
		for(int b = 0; b < hotspotsTotal; b++) {
			hotspotId = readInt(fin);
			dout << "    hotspot: " << hotspotId << endl;
			HMI i = all_hotspots.find(hotspotId);
			if(i == all_hotspots.end()) {
				dout << "ERROR- Could not find hotspot" << endl;
				exit(1);
			}
			Hotspot *h = i->second;
			s->hotspots.insert(make_pair<int, Hotspot*>(hotspotId, h));

			if(s->id == currentSceneId) {
				loadHotspotAnimData(h);
			}
		}

		s->walkMask = new WalkMask;
		if(s->walkMask->readWalkMask(fin)) {
			dout << "ERROR- could not load walkmask" << endl;
			return 1;
		}

		ResortHotspots(s);

		all_scenes.insert(make_pair<int, Scene*>(s->id, s));
		if(s->id == currentSceneId) {
			current_scene = s;
		}
	}

	if(!current_scene) {
		dout << "ERRRO- no current scene" << endl;
		return 1;
	}

	current_scene->gfx = LoadAnimation(current_scene->gfxFile, true, false);
	if(!current_scene->gfx) {
		dout << "ERROR- Could not load gfx for scene: " << current_scene->id << endl;
		exit(1);
	}
	readAnimationState(fin, current_scene->gfx);

	current_scene->w = current_scene->walkMask->getWidth();
	current_scene->h = current_scene->walkMask->getHeight();

	current_scene->sound = loadSoundCollection(current_scene->soundFile, SOUND_TYPE_SCENE);
	if(current_scene->sound) {
		loadSoundCollectionState(fin, current_scene->sound);
		soundPlayer.loadSceneSoundState(fin, current_scene->sound);
	}
	fclose(fin);
	dout << " End loadScenes" << endl;
	
	return 0;
}

int readInventory(string filePrefix) {
	dout << " loadInventory" << endl;	
	
	string fileName = filePrefix + "inventory.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadInventory: Could not open " << fileName << " for reading." << endl;
		return 1;
	}
	int totalItems = readInt(fin);
	dout << "  total items: " << totalItems << endl;
	int a;
	for(a = 0; a < totalItems; a++) {
		InventoryItem *item = new InventoryItem;
		item->id = readInt(fin);
		dout << "  item: " << item->id << endl;
		item->unselectedGfxFile = readString(fin);
		item->selectedGfxFile = readString(fin);
		item->long_name = readString(fin);
		item->short_name = readString(fin);

		int totalScripts = readInt(fin);
		for(int b = 0; b < totalScripts; b++) {
			int itemId = readInt(fin);
			int scriptId = readInt(fin);
			item->scripts.insert(make_pair<int, int>(itemId, scriptId));
		}
		all_inventory_items.insert(make_pair<int, InventoryItem*>(item->id, item));
	}

	int inventoryTotal = readInt(fin);
	for(a = 0; a < inventoryTotal; a++) {
		int itemId = readInt(fin);
		dout << "   Adding to inventory, item: " << itemId << endl;
		IIMI i = all_inventory_items.find(itemId);
		if(i == all_inventory_items.end()) {
			dout << "ERROR- Wanted to add invalid inventory item: " << itemId << endl;
			exit(1);
		}
		InventoryItem *item = i->second;
		LoadInventoryItemData(item);
		readAnimationState(fin, item->unselectedGfx);
		if(item->selectedGfxFile.length() > 0) {
			readAnimationState(fin, item->selectedGfx);
		}
		inventory.insert(make_pair<int, InventoryItem*>(itemId, item));
	}
	currentInventorySkin->load(fin);
	fclose(fin);
	dout << " End loadInventory" << endl;	
	return 0;
}

int readDialogs(string filePrefix) {
	dout << " loadDialogs" << endl;	
	
	string fileName = filePrefix + "dialogs.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadDialogs: Could not open " << fileName << " for reading." << endl;
		return 1;
	}
	int dlgTotal = readInt(fin);
	dout << "  total dialogs: " << dlgTotal << endl;

	int tmp;

	for(int a = 0; a < dlgTotal; a++) {
		Dialog *dlg = new Dialog;
		dlg->id = readInt(fin);
		dout << "   dialog id: " << dlg->id << endl;
		dlg->firstQuestionId = readInt(fin);
		int questionTotal = readInt(fin);
		for(int b = 0; b < questionTotal; b++) {
			DialogQuestion q;
			q.id = readInt(fin);
			q.enabledAnswers = readInt(fin);
			q.inScriptId = readInt(fin);
			q.outScriptId = readInt(fin);
			q.nextDialogQuestion = readInt(fin);
			q.questionSelectMode = readInt(fin);
			q.currentQuestionIdx = readInt(fin);
			int totalQuestionsTexts = readInt(fin);
			for(tmp = 0; tmp < totalQuestionsTexts; tmp++) {
				q.questions.push_back(readString(fin));
				q.soundFiles.push_back(readString(fin));
				dout << "    question id: " << q.id << " [" << tmp << "] '" << q.questions[tmp] << "'" << endl;
			}
			
			int answerTotal = readInt(fin);
			for(int c = 0; c < answerTotal; c++) {
				DialogAnswer answer;
				answer.id = readInt(fin);
				answer.enabled = (readInt(fin) > 0);
				answer.nextQuestionId = readInt(fin);
				answer.visited = (readInt(fin) > 0);
				answer.inScriptId = readInt(fin);
				answer.outScriptId = readInt(fin);
				answer.answerSelectMode = readInt(fin);
				answer.currentAnswerIdx = readInt(fin);
				int totalAnswerTexts = readInt(fin);
				for(tmp = 0; tmp < totalAnswerTexts; tmp++) {
					answer.answerChoose.push_back(readString(fin));
					answer.answerReal.push_back(readString(fin));
					answer.soundFiles.push_back(readString(fin));
					if(answer.answerChoose[tmp] != answer.answerReal[tmp]) {
						dout << "     answer id: " << answer.id << " [" << c << ", " << tmp << "], choose: '" << answer.answerChoose[tmp] << "', real: '" << answer.answerReal[tmp] << "'" << endl;
					} else {
						dout << "     answer id: " << answer.id << " [" << c << ", " << tmp << "] '" << answer.answerChoose[tmp] << "'"  << endl;
					}
				}
				q.answers.push_back(answer);
			}
			dlg->questions.insert(make_pair<int, DialogQuestion>(q.id, q));
		}
		dout << "  dialog loaded: " << dlg->id << endl;
		all_dialogs.insert(make_pair<int, Dialog*>(dlg->id, dlg));
	}
	dout << " End loadDialogs" << endl;	
	fclose(fin);
	return 0;
}


int readInt(FILE *fin) {
	int tmp = -1;
	if(!fread(&tmp, sizeof(int), 1, fin)) {
		dout << "ERRROR- readInt failed: " << strerror(errno)<< endl;
		exit(1);	
	}
	return tmp;
}

string readString(FILE *fin) {
	static char tmpBuf[1024];
	memset(tmpBuf, 0 ,1024);
	int size = readInt(fin);
	if(size > 1023) {
		dout << "ERROR- readString failed: string size >= 1024 bytes" << endl;
		exit(1);
	}
	if((size > 0) && (fread(tmpBuf, size, 1, fin) != 1)) {
		dout << "ERRROR- readString failed: " << strerror(errno)<< endl;
		exit(1);	
	}
	return string(tmpBuf);
}

double readDouble(FILE *fin) {
	double tmp = 0.0;
	if(!fread(&tmp, sizeof(double), 1, fin)) {
		dout << "ERRROR- readDouble failed: " << strerror(errno)<< endl;
		exit(1);	
	}
	return tmp;	
}

int readMusic(string filePrefix) {
	dout << " loadMusic" << endl;	
	
	string fileName = filePrefix + "music.dat";
	FILE *fin = fopen(fileName.c_str(), "rb");
	if(!fin) {
		dout << "ERROR- loadMusic: Could not open " << fileName << " for reading." << endl;
		return 1;
	}
	musicPlayer.loadMusic(fin);
	dout << " End loadMusic" << endl;	
	fclose(fin);
	return 0;
}

int readAnimationTransformState(FILE *fin, Animation *anim) {
	int idx = readInt(fin);	
	if(idx == -1) {
		anim->currentTransform = NULL;
		anim->currentTransformIdx = 1;
		anim->currentTransformLoops = 0;
		return 0;
	}
	anim->currentTransformIdx = idx;
	anim->currentTransform = anim->transforms[idx];
	anim->currentTransformLoops = readInt(fin);

	anim->currentTransform->currentX = readInt(fin);
	anim->currentTransform->currentY = readInt(fin);
	anim->currentTransform->startTime = readInt(fin);
	anim->currentTransform->endTime = readInt(fin);
	anim->currentTransform->lastUpdate = readInt(fin);
	anim->currentTransform->length = readInt(fin);
	return 0;
}

