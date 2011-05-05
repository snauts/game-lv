#include "game.h"
#include "structs.h"

int saveAnimationState(FILE *fout, Animation *anim);
int saveAnimationTransformState(FILE *fout, Animation *anim);
int saveScript(FILE *fout, Script *script);


int saveStateMachine(FILE *fout, state_machine<string, int, debug_out> &m);
int saveMainCharacter(string filePrefix);
int saveAllScripts(string filePrefix);
int saveGameState(string filePrefix);
int saveStates(string filePrefix);
int saveHotspots(string filePrefix);
int saveScenes(string filePrefix);
int saveInventory(string filePrefix);
int saveDialogs(string filePrefix);
int saveMusic(string filePrefix);

int saveGame(string filePrefix) {

	dout << "--- saveGame (started in: " << current_time << ")" << endl;
	dout << "Saving game in '" << filePrefix << "'" << endl;
	soundPlayer.pauseAll();
	saveGameState(filePrefix);
	saveMainCharacter(filePrefix);
	saveAllScripts(filePrefix);
	saveStates(filePrefix);
	saveHotspots(filePrefix);
	saveScenes(filePrefix);
	saveInventory(filePrefix);
	saveDialogs(filePrefix);
	saveMusic(filePrefix);
	soundPlayer.resumeAll();
	dout << "--- End saveGame (ended in: " << current_time << ")" << endl << endl;
	return 0;
}

int saveDialogs(string filePrefix) {
	dout << " saveDialogs" << endl;
	string fileName = filePrefix + "dialogs.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveDialogs: Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	// Save dialogs.
	saveInt(fout, all_dialogs.size());
	DMI i = all_dialogs.begin();
	Dialog *d;
	map<int, DialogQuestion>::iterator dqi;
	unsigned tmp;
	int c;
	while(i != all_dialogs.end()) {
		d = i->second;
		dout << "  save dialog, id : " << d->id << endl;
		saveInt(fout, d->id);
		saveInt(fout, d->firstQuestionId);

		// Save questions
		saveInt(fout, d->questions.size());
		dqi = d->questions.begin();
		while(dqi != d->questions.end()) {
			DialogQuestion &q = dqi->second;

			saveInt(fout, q.id);
			saveInt(fout, q.enabledAnswers);
			saveInt(fout, q.inScriptId);
			saveInt(fout, q.outScriptId);
			saveInt(fout, q.nextDialogQuestion);
			saveInt(fout, q.questionSelectMode);
			saveInt(fout, q.currentQuestionIdx);
			saveInt(fout, q.questions.size());
			for(c = 0; c < q.questions.size(); c++) {
				saveString(fout, q.questions[c]);
				saveString(fout, q.soundFiles[c]);
			}
			// Save answers.
			saveInt(fout, q.answers.size());
			for(tmp = 0; tmp < q.answers.size(); tmp++) {
				DialogAnswer &a = q.answers[tmp];
				saveInt(fout, a.id);				
				saveInt(fout, a.enabled);
				saveInt(fout, a.nextQuestionId);
				saveInt(fout, a.visited);
				saveInt(fout, a.inScriptId);
				saveInt(fout, a.outScriptId);
				saveInt(fout, a.answerSelectMode);
				saveInt(fout, a.currentAnswerIdx);
				saveInt(fout, a.answerChoose.size());
				for(c = 0; c < a.answerChoose.size(); c++) {
					saveString(fout, a.answerChoose[c]);
					saveString(fout, a.answerReal[c] );
					saveString(fout, a.soundFiles[c]);
				}
			}
			dqi++;
		}

		i++;
	}
	
	dout << " End saveDialogs" << endl << endl;
	
	fclose(fout);
	return 0;
}

int saveInventory(string filePrefix) {
	dout << " saveInventory" << endl;
	string fileName = filePrefix + "inventory.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveInventory: Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	saveInt(fout, all_inventory_items.size());
	IIMI i = all_inventory_items.begin();
	InventoryItem *item;
	while(i != all_inventory_items.end()) {
		item = i->second;
		dout << "  save item, id : " << item->id << endl;
		saveInt(fout, item->id);
		saveString(fout, item->unselectedGfxFile);
		saveString(fout, item->selectedGfxFile);
		saveString(fout, item->long_name);
		saveString(fout, item->short_name);

		saveInt(fout, item->scripts.size());
		multimap<int, int>::iterator si = item->scripts.begin();
		while(si != item->scripts.end()) {
			saveInt(fout, si->first); // item id.
			saveInt(fout, si->second); // script id
			si++;
		}
		

		i++;
	}


	// Save all item id's which are currently in inventory.
	saveInt(fout, inventory.size());
	i = inventory.begin();
	while(i != inventory.end()) {
		saveInt(fout, i->first);
		saveAnimationState(fout, i->second->unselectedGfx);
		if(i->second->selectedGfxFile.length() > 0) {
			saveAnimationState(fout, i->second->selectedGfx);
		}
		i++;
	}

	currentInventorySkin->save(fout);
	

	fclose(fout);

	dout << " End saveInventory" << endl << endl;
	
	return 0;
}

int saveScenes(string filePrefix) {
	dout << " saveScenes" << endl;
	string fileName = filePrefix + "scenes.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveScenes: Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	saveInt(fout, current_scene->id);

	saveInt(fout, all_scenes.size());
	SCMI i = all_scenes.begin();
	Scene *s;
	while(i != all_scenes.end()) {
		s = i->second;
		dout << "  save scene, id : " << s->id << endl;
		saveInt(fout, s->id);
		saveString(fout, s->gfxFile);
		saveString(fout, s->maskFile);
		saveString(fout, s->soundFile);

		HMI hi = s->hotspots.begin();
		saveInt(fout, s->hotspots.size());
		while(hi != s->hotspots.end()) {
			saveInt(fout, hi->first);
			hi++;
		}

		s->walkMask->saveWalkMask(fout);

		i++;
	}

	saveAnimationState(fout, current_scene->gfx);
	if(current_scene->sound) {
		saveSoundCollectionState(fout, current_scene->sound);
		soundPlayer.saveSceneSoundState(fout);
	}
	
	fclose(fout);
	
	dout << " End saveScenes" << endl << endl;
	return 0;
}


int saveHotspots(string filePrefix) {
	dout << " saveHotspots" << endl;
	string fileName = filePrefix + "hotspots.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveHotspots:Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	saveInt(fout, all_hotspots.size());

	HMI i = all_hotspots.begin();
	Hotspot *h;
	HotspotAnim *ha;
	while(i != all_hotspots.end()) {
		h = i->second;

		dout << "  save hotspot, id : " << h->id << endl;
		saveInt(fout, h->id);
		saveInt(fout, h->anim_id);
		saveInt(fout, h->depth);
		saveInt(fout, h->enabled);

		saveInt(fout, h->anims.size());
		map<int, HotspotAnim*>::iterator animIterator = h->anims.begin();
		while(animIterator != h->anims.end()) {
			ha = animIterator->second;
			saveInt(fout, ha->id);
			saveInt(fout, ha->x);
			saveInt(fout, ha->y);
			saveInt(fout, ha->restartSoundOnAnimLoop);
			saveString(fout, ha->description);
			saveString(fout, ha->gfxName);
			saveString(fout, ha->maskFile);
			saveString(fout, ha->soundFile);
			animIterator++;
		}
		saveAnimationState(fout, h->anim);

		saveInt(fout, h->scripts.size());
		multimap<int, int>::iterator si = h->scripts.begin();
		while(si != h->scripts.end()) {
			saveInt(fout, si->first); // item id.
			saveInt(fout, si->second); // script id
			si++;
		}
		i++;
	}

	map<unsigned, HotspotSoundInfo> m = soundPlayer.getCurrentlyPlayingHotspots();
	map<unsigned, HotspotSoundInfo>::iterator mi = m.begin();
	saveInt(fout, m.size());
	while(mi != m.end()) {
		HotspotSoundInfo info = mi->second;
		HMI hmi = all_hotspots.find(info.hotspotId);
		if(hmi == all_hotspots.end()) {
			dout << "ERROR- when saving hotspot sounds, unknown hotspot: " << info.hotspotId << endl;
			exit(1);
		}
		Hotspot *h = hmi->second;
		saveInt(fout, h->id);
		if(h->sound) {
			saveInt(fout, 1);
			saveSoundCollectionState(fout, h->sound);
			soundPlayer.saveHotspotSoundState(fout, h->id);
		} else {
			saveInt(fout, 0);
		}
		mi++;
	}
	
	fclose(fout);
	
	dout << " End saveHotspots" << endl << endl;
	
	return 0;
	
}

int saveStates(string filePrefix) {
	dout << " saveStates" << endl;
	string fileName = filePrefix + "state_machine.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveStates::Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	saveStateMachine(fout, game_states);

	fclose(fout);

	dout << " End saveStates" << endl  << endl;

	return 0;
}

int saveGameState(string filePrefix) {
	dout << " saveGameState" << endl;
	string fileName = filePrefix + "game.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveGameState::Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	saveInt(fout, current_time);
	saveInt(fout, current_game_state);
	saveInt(fout, state_var1);
	saveInt(fout, state_var2);

	dout << "  time: " << current_time << endl;
	dout << "  state: " << current_game_state << endl;
	dout << "  state_var1: " << state_var1 << endl;
	dout << "  state_var2: " << state_var2 << endl;

	if(currentDialog) {
		saveInt(fout, currentDialog->id);
		saveInt(fout, currentDialogQuestion->id);
		if(currentDialogAnswer) {
			for(unsigned tmp = 0; tmp < currentDialogQuestion->answers.size(); tmp++) {
				if(currentDialogAnswer == &(currentDialogQuestion->answers[tmp])) {
					saveInt(fout, tmp);
					break;
				}
			}
		} else {
			saveInt(fout, -1);
		}
		if((state_var1 == DIALOG_SHOW_QUESTION) || (state_var1 == DIALOG_SHOW_ANSWER)) {
			if(soundPlayer.isDialogPlaying()) {
				saveInt(fout, 1);
				soundPlayer.saveDialogSoundState(fout);
			} else {
				saveInt(fout, 0);
			}
		}
	} else {
		saveInt(fout, -1);
	}
	saveInt(fout, currentViewX);
	saveInt(fout, currentViewY);

	// Save current visible text strings.
	saveInt(fout, onscreen_text.size());
	list<TextItem>::iterator iter = onscreen_text.begin();
	while(iter != onscreen_text.end()) {
		TextItem &i = *iter;

		saveString(fout, i.text);
		saveInt(fout, i.expireTime);
		saveInt(fout, i.fontId);
		iter++;
	}

	fclose(fout);
	dout << " End saveGameState" << endl << endl;
	return 0;
}

int saveAllScripts(string filePrefix) {

	dout << " saveAllScripts" << endl;
	string fileName = filePrefix + "scripts.dat";
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveAllScripts::Could not open " << fileName << " for writing." << endl;
		return 1;
	}

	saveInt(fout, all_scripts.size());
	
	SMI i = all_scripts.begin();
	Script *s;
	while(i != all_scripts.end()) {
		s = i->second;
		dout << "  save script, id : " << s->id << endl;
		saveScript(fout, s);
		i++;
	}

	// Save script sounds.
	map<int, SoundCollection*> m = soundPlayer.getCurrentlyPlayingScriptSounds();
	map<int, SoundCollection*>::iterator channelIter = m.begin();
	map<int, SoundCollection*>::iterator soundIter;
	SoundCollection *sound;
	int channel;
	saveInt(fout, m.size());
	while(channelIter != m.end()) {
		channel = channelIter->first;
		sound = channelIter->second;
		soundIter = scriptSounds.find(sound->id);
		if(soundIter == scriptSounds.end()) {
			dout << "ERROR- while saving script sound, could not find sound which was played on channel: "
				<< channel << " with id: " << sound->id << endl;
			exit(1);
		}
		saveInt(fout, channel);
		saveInt(fout, sound->id);
		saveSoundCollectionState(fout, sound);
		soundPlayer.saveScriptSoundChannel(fout, channel);
		channelIter++;
	}

	fclose(fout);
	dout << " End saveAllScripts" << endl << endl;
	return 0;	
}

int saveMainCharacter(string filePrefix) {
	dout << " saveMainCharacter" << endl;
	Character *c = main_character;
	string fileName = filePrefix + "character.dat";

	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveMainCharacter::Could not open " << fileName << " for writing." << endl;
		return 1;
	}
	
	saveInt(fout, c->state);
	saveInt(fout, c->prevState);
	saveInt(fout, current_time - c->lastUpdate);
	saveInt(fout, c->visible);
	saveInt(fout, c->currentAnimationSetId);

	// Save all animation sets.
	saveInt(fout, c->animationSet.size()); // Total count of animations sets.
	map<int, CharacterAnimationSet*>::iterator animSetIter = c->animationSet.begin();
	while(animSetIter != c->animationSet.end()) { // Iterate trough all anim sets
		CharacterAnimationSet *currentAnimSet = animSetIter->second;
		saveInt(fout, currentAnimSet->id);
		saveString(fout, currentAnimSet->characterName);
		saveInt(fout, currentAnimSet->anims.size()); // Animation count in animation set.
		map<int, CharacterAnimation*>::iterator currentAnimIter = currentAnimSet->anims.begin();
		while(currentAnimIter != currentAnimSet->anims.end()) { // Iterate trough all anims
			CharacterAnimation *charAnim = currentAnimIter->second;
			saveInt(fout, charAnim->id);
			saveInt(fout, charAnim->moveSpeed);
			saveString(fout, charAnim->soundFile);
			saveInt(fout, charAnim->directions);
			saveInt(fout, charAnim->animationsPerDirection);
			if(charAnim->directions != charAnim->anims.size()) {
				dout << "ERROR- character animation out of sync 1" << endl;
				exit(1);
			}

			for(int tmp = 0; tmp < charAnim->directions; tmp++) { // Iterate through all one-way anims
				CharOneWayAnim *oneWayAnim = charAnim->anims[tmp];
				if(oneWayAnim->gfxFile.size() != charAnim->animationsPerDirection) {
					dout << "ERROR- character animation out of sync 2" << endl;
					exit(1);
				}
				for(int tmpval = 0; tmpval < charAnim->animationsPerDirection; tmpval++) {
					saveString(fout, oneWayAnim->gfxFile[tmpval]);
				}
				
			}
			currentAnimIter++;
		}
		saveInt(fout, currentAnimSet->mappedAnimationIds.size());
		map<int, int>::iterator mi = currentAnimSet->mappedAnimationIds.begin();
		while(mi != currentAnimSet->mappedAnimationIds.end()) {
			saveInt(fout, mi->first);
			saveInt(fout, mi->second);
			mi++;
		}
		
		animSetIter++;
	}
	

	// Current walking state also should be saved...

	// Lets save current animation (c->current_anim) id & direction & current state from current animation set.
	map<int, CharacterAnimation*>::iterator currentAnimIter = c->anims.begin();
	while(currentAnimIter != c->anims.end()) {
		CharacterAnimation *charAnim = currentAnimIter->second;
		
		// Nevar likt aiz for, savaadaak var sanaakt ka ++ tiek taisiits,
		// .end() iteratoram.
		currentAnimIter++;
		for(int tmp = 0; tmp < charAnim->anims.size(); tmp++) {
			CharOneWayAnim *oneWayAnim = charAnim->anims[tmp];
			if(oneWayAnim->anim == c->current_anim) {
				saveInt(fout, charAnim->id); // Save current animation id.
				saveInt(fout, tmp); // Save current animation direction.
				saveInt(fout, c->currentAnimSize);
				for(int tmpval = 0; tmpval < c->currentAnimSize; tmpval++) {
					saveAnimationState(fout, c->current_anim[tmpval]); // save current animation state.
				}
				currentAnimIter = c->anims.end();
				if(charAnim->sound) {
					saveSoundCollectionState(fout, charAnim->sound);
					soundPlayer.saveCharacterSoundState(fout);
				}
				break;
			}
		}
	}


	// Save suspended script, if there is one.
	if(c->suspended_script) {
		saveInt(fout, c->suspended_script->id);
		int tmp;
		for(tmp = 0; tmp < c->suspended_script->actions.size(); tmp++) {
			if(c->suspended_action == c->suspended_script->actions[tmp]) {
				saveInt(fout, tmp);
				break;
			} 
		}

		// save suspended action handler index.
		for(tmp = 0; tmp < SUSPENDABLE_ACTIONS; tmp++) {
			if(c->isActionComplete == suspendedScriptActionHandlers[tmp]) {
				saveInt(fout, tmp);
				break;
			}
		}

		for(tmp = 0; tmp < SCRIPT_ACTION_STATIC_DATA_SIZE; tmp++) {
			saveInt(fout, ScriptAction::tmpval[tmp]);
		}
	} else {
		saveInt(fout, -1);
	}

	CoordinatePoint::saveCoordinatePoint(fout, c->coord);

	fclose(fout);
	dout << " End saveMainCharacter" << endl << endl;
	return 0;
}


int saveInt(FILE *fout, int val) {
	return fwrite(&val, sizeof(int), 1, fout);
}

int saveString(FILE *fout, const string &val) {
	saveInt(fout, val.length());
	return fwrite(val.c_str(), val.length(), 1, fout);
}

int saveDouble(FILE *fout, double val) {
	return fwrite(&val, sizeof(double), 1, fout);
}

int saveAnimationState(FILE *fout, Animation *anim) {
	dout << "SaveAnimState currentFrame: " << anim->currentFrame 
		<< ", last update: " << anim->lastUpdate
		<< ", current time: " << current_time << endl;
	saveInt(fout, anim->currentBlockIdx);
	saveInt(fout, anim->currentFrame);
	saveInt(fout, anim->currentLoops);
	saveInt(fout, anim->lastUpdate);
	saveInt(fout, anim->totalBlocks);
	saveInt(fout, anim->useVideoMemory);
	saveInt(fout, anim->isImageDataLoaded);
	saveAnimationTransformState(fout, anim);
	return 0;
}

int saveScript(FILE *fout, Script *script) {
	saveInt(fout, script->id);
	saveInt(fout, script->execute_times);
	saveInt(fout, script->type);
	saveInt(fout, script->param1);
	saveInt(fout, script->param2);
	ScriptAction *action;
	saveInt(fout, script->actions.size());
	for(int tmp = 0; tmp < script->actions.size(); tmp++) {
		action = script->actions[tmp];
		saveInt(fout, action->action);
		saveInt(fout, action->param1);
		saveInt(fout, action->param2);
		saveInt(fout, action->param3);
		saveInt(fout, action->param4);
		saveString(fout, action->str);
	}
	return 0;
}

int saveStateMachine(FILE *fout, state_machine<string, int, debug_out> &m) {
	const map<string, int> &states = m.getStatesRef();
	state_machine<string, int, debug_out>::CSI i = states.begin();
	saveInt(fout, states.size());
	dout << m;
	while(i != states.end()) {
		saveString(fout, i->first);
		saveInt(fout, i->second);
		i++;
	}
	return 0;
}

int saveMusic(string filePrefix) {
	dout << " saveMusic" << endl;
	string fileName = filePrefix + "music.dat";
	
	FILE *fout = fopen(fileName.c_str(), "wb");
	if(!fout) {
		dout << "ERROR- saveMusic::Could not open " << fileName << " for writing." << endl;
		return 1;
	}
	musicPlayer.saveMusic(fout);

	fclose(fout);
	dout << " End saveMusic" << endl;
	return 0;
}

int saveAnimationTransformState(FILE *fout, Animation *anim) {
	AnimationTransform *tr = anim->currentTransform;
	if(tr) {
		saveInt(fout, anim->currentTransformIdx);
		saveInt(fout, anim->currentTransformLoops);
	} else {
		saveInt(fout, -1);
		return 0;
	}

	saveInt(fout, tr->currentX);
	saveInt(fout, tr->currentY);
	saveInt(fout, tr->startTime);
	saveInt(fout, tr->endTime);
	saveInt(fout, tr->lastUpdate);
	saveInt(fout, tr->length);
	return 0;
}
