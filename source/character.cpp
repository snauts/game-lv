#include "game.h"
#include "sound.h"

unsigned freeCharacterData(Character *c) {
	map<int, CharacterAnimation*>::iterator i = c->anims.begin();
	CharacterAnimation* a;
	CharOneWayAnim *o;
	while(i != c->anims.end()) {
		a = i->second;
		if(a->sound) {
			delete a->sound;			
			a->sound = NULL;
		}
		for(unsigned c = 0; c < a->anims.size(); c++) {
			o = a->anims[c];
			for(unsigned tmpVal = 0; tmpVal < a->animationsPerDirection; tmpVal++) {
				delete o->anim[tmpVal];
			}
			delete[] o->anim;
			o->anim = NULL;
		}
		i++;
	}
	c->anims.clear();
	c->current_anim = NULL;
	c->currentAnimationSetId = -1;
	c->characterName = "";
	return 0;
}

unsigned loadCharacterData(Character* c) {
	map<int, CharacterAnimation*>::iterator i = c->anims.begin();
	CharacterAnimation* a;
	CharOneWayAnim* o;
	while(i != c->anims.end()) {
		a = i->second;
		a->sound = loadSoundCollection(a->soundFile, SOUND_TYPE_CHARACTER);
		for(unsigned b = 0; b < a->anims.size(); b++) {
			o = a->anims[b];

			if(o->gfxFile.size() != a->animationsPerDirection) {
				dout << "ERROR- animationsPerDirections != OneWayAnim.gfxFile.size()" << endl;
				exit(1);
				return 1;
			}

			o->anim = new Animation*[o->gfxFile.size()];
			for(unsigned tmpVal = 0; tmpVal < o->gfxFile.size(); tmpVal++) {
				if(!(o->anim[tmpVal] = LoadAnimation(o->gfxFile[tmpVal], false))) {
					dout << "ERROR- could not load character data, animation: " << o->gfxFile[tmpVal] << endl;
					exit(1);
					return 1;
				}
			}
		}
		i++;
	}
	return 0;
}

unsigned setCharacterAnimationSet(int animationSetId) {
	if(main_character->currentAnimationSetId == animationSetId) {
		dout << " current character animation set already is: " << animationSetId << endl;
		return 0;
	}
	map<int, CharacterAnimationSet*>::iterator i;

	

	if(main_character->currentAnimationSetId >= 0)	{
		i = main_character->animationSet.find(main_character->currentAnimationSetId);
		CharacterAnimationSet *tmpSet = i->second;
		tmpSet->mappedAnimationIds = main_character->mappedAnimationIds;
		freeCharacterData(main_character);
		soundPlayer.freeSoundData(SOUND_TYPE_CHARACTER);
	}
	
	i = main_character->animationSet.find(animationSetId);
	if(i == main_character->animationSet.end()) {
		// Animation set with this id- does not exists.
		dout << "ERROR- Wanted to set non-existing character animation set: " << animationSetId << endl;
		exit(1);
	}
	
	CharacterAnimationSet *set = i->second;
	main_character->characterName = set->characterName;
	main_character->anims = set->anims;
	main_character->currentAnimationSetId = set->id;
	main_character->mappedAnimationIds = set->mappedAnimationIds;
	loadCharacterData(main_character);
	SetCurrentCharacterAnimation(main_character); // <-- ??? NAFIG????
	return 0;
}

void setCharacterState(int state) {
	Character *c = main_character;
	c->prevState = main_character->state;
	c->state = state;

	// Set character speed
	map<int, int>::iterator idi = c->mappedAnimationIds.find(c->state);
	map<int, CharacterAnimation*>::iterator i;
	if(idi != c->mappedAnimationIds.end())	 {
		i = c->anims.find(idi->second);
		if(i == c->anims.end()) {
			dout << "ERROR- Want to set unknown character animation: " << idi->second << " as result of mapping from: "
				<< c->state << endl;
			exit(1);
		}
	} else {
		i = c->anims.find(c->state);
		if(i == c->anims.end()) {
			dout << "ERROR- Want to set unknown character animation: " << c->state << endl;
			exit(1);
		}
	}
	CharacterAnimation* anim = i->second;
	c->current_speed = anim->moveSpeed;
}

void SetCurrentCharacterAnimation(Character* c, bool shouldRestartSound, 
								  bool synchroniseFrames) {
	map<int, int>::iterator idi = c->mappedAnimationIds.find(c->state);
	map<int, CharacterAnimation*>::iterator i;
	if(idi != c->mappedAnimationIds.end())	 {
		i = c->anims.find(idi->second);
		if(i == c->anims.end()) {
			dout << "ERROR- Want to set unknown character animation: " << idi->second << " as result of mapping from: "
				 << c->state << endl;
			exit(1);
		}
	} else {
		i = c->anims.find(c->state);
		if(i == c->anims.end()) {
			dout << "ERROR- Want to set unknown character animation: " << c->state << endl;
			exit(1);
		}
	}
	CharacterAnimation* anim = i->second;

	switch(c->state) {
	case(CHARACTER_WALK):
	case(CHARACTER_STAND):
	case(CHARACTER_TALK):
		{
			double rad_per_anim = ((2.0 * PI_COEF) / (double)anim->anims.size());
			double new_dir = c->coord.angle + (rad_per_anim / 2.0);
			if(new_dir < 0.0) {
				new_dir += PI_COEF * 2.0;
			} else if(new_dir >= (2.0 * PI_COEF)) {
				new_dir -= PI_COEF * 2.0;
			}
			int current_anim = (int)(new_dir / rad_per_anim) % anim->directions;
			Animation **oldAnim = c->current_anim;
			CharOneWayAnim* o = anim->anims[current_anim];
			c->current_anim = o->anim;
			c->currentAnimSize = anim->animationsPerDirection;
			c->current_speed = anim->moveSpeed;
			
			if((c->state == c->prevState) && (c->state == CHARACTER_WALK) && (oldAnim == c->current_anim)) {
				// Was walking already in this direction, so there is no need to reset walk animation.
			} else if((c->state == CHARACTER_WALK) && synchroniseFrames) {
				if(oldAnim != c->current_anim) {
					for(unsigned tmpVal = 0; tmpVal < anim->animationsPerDirection; tmpVal++) {
						Animation *prevAnim = oldAnim[tmpVal];
						Animation *curAnim = c->current_anim[tmpVal];
						resetAnimation(curAnim);
						curAnim->currentBlockIdx = prevAnim->currentBlockIdx;
						curAnim->currentFrame = prevAnim->currentFrame;
						curAnim->currentBlock = curAnim->blocks[curAnim->currentBlockIdx];
					}
				}
			} else {
				for(unsigned tmpVal = 0; tmpVal < anim->animationsPerDirection; tmpVal++) {
					//				c->current_anim[tmpVal]->lastUpdate = current_time;
					resetAnimation(c->current_anim[tmpVal]);
				}
			}

			if(((c->prevState != c->state) && (c->visible)) || (shouldRestartSound)) {
				soundPlayer.playCharacterSound(anim->sound);
			} 
			break;
		}
	}
}

CharacterAnimation* LoadCharacterAnimation(FILE* fin) {
	if(!IsNextString(fin, "character_animation")) {
		dout << "no 'character_animation' tag found" << endl;
		return NULL;
	}
	CharacterAnimation* anim = new CharacterAnimation;
	anim->id = LoadInt(fin);
	anim->directions = LoadInt(fin);
	anim->animationsPerDirection = LoadInt(fin);
	anim->moveSpeed = LoadInt(fin);
	anim->soundFile = LoadString(fin);
	convertPathInPlace(anim->soundFile);
	CharOneWayAnim* one_way;
	string gfxFile;
	for(unsigned a = 0; a < anim->directions; a++) {
		if(IsNextString(fin, "end_character_animation", false))
			break;
		one_way = new CharOneWayAnim;
		for(unsigned b = 0; b < anim->animationsPerDirection; b++) {
			gfxFile = LoadString(fin);
			one_way->gfxFile.push_back(gfxFile);
		}
		anim->anims.push_back(one_way);
	}
	if(!anim->directions) {
		dout << "ERROR- character animation: " << anim->id << " has 0 directions" << endl;
//		delete anim;
//		return NULL;
		exit(1);
	}
	if(anim->anims.size() != anim->directions) {
		dout << "ERROR- animation has only: " << anim->anims.size() << " animations from " << anim->directions << endl;
//		delete anim;
//		return NULL;
		exit(1);
	}
	if(!IsNextString(fin, "end_character_animation")) {
		dout << "ERROR- no 'end_character_animation' tag found" << endl;
//		delete anim;
//		return NULL;
		exit(1);
	}
	return anim;
}


Character* LoadCharacter(const string& filename) {
	dout << "--- LoadCharacter" << endl;
	dout << " file: " << filename << endl;
	FILE* fin = fopen(filename.c_str(), "r");
	if(!fin) {
		dout << " could not open file " << endl;
		exit(1);
	}
	Character* c;
	CharacterAnimation* anim;
	if(!IsNextString(fin, "character")) {
		dout << " no 'character' tag found" << endl;
		fclose(fin);
		return NULL;
	}
	c = new Character;
	string name;
	map<int, CharacterAnimation*> anims;
	int animSetId;
	int firstAnimSet;

	firstAnimSet = LoadInt(fin);
	while(IsNextString(fin, "character_animation_set")) {
		anims.clear();
		animSetId = -1;
		name = "";

		animSetId = LoadInt(fin);
		name = LoadString(fin, true);
		dout << "processing animation set: " << animSetId << endl;

		while(IsNextString(fin, "character_animation", false)) {
			anim = LoadCharacterAnimation(fin);
			if(anim) {
				anims.insert(make_pair<unsigned, CharacterAnimation*>(anim->id, anim));
				dout << " animation: " << anim->id << " added" << endl;
			}
			else {
				dout << "ERROR- tried to load invalid animation" << endl;
				exit(1);
//				delete c;
//				return NULL;
			}
		}
		if(!IsNextString(fin, "end_character_animation_set")) {
			dout << " no 'end_character_animation_set' tag found" << endl;
			delete c;
			fclose(fin);
			return NULL;
		}
		if(c->animationSet.find(animSetId) != c->animationSet.end()) {
			dout << "WARNING- animation set with id: " << animSetId << " already exists" << endl;
			continue;
		}
		CharacterAnimationSet *animSet = new CharacterAnimationSet;
		animSet->id = animSetId;
		animSet->characterName = name;
		animSet->anims = anims;
		c->animationSet.insert(make_pair<int, CharacterAnimationSet*>(animSetId, animSet));
		if(firstAnimSet == animSetId) {
			c->anims = anims;
			c->currentAnimationSetId = animSetId;
			c->characterName = name;
		}
	}
	if(!IsNextString(fin, "end_character")) {
		dout << " no 'character' tag found" << endl;
		delete c;
		fclose(fin);
		return NULL;
	}
	if(c->currentAnimationSetId == -1) {
		dout << "ERROR- Invalid default animation set value: " << firstAnimSet << endl;
		exit(1);
//		delete c;
//		return NULL;
	}

	fclose(fin);

	loadCharacterData(c);
	SetCurrentCharacterAnimation(c);
	dout << "--- EndLoadCharacter" << endl << endl;;
	return c;
}

void UpdateCharacter() {
	Character* c = main_character;
	int err;
	double lastAngle = c->coord.angle;
	if(c->state == CHARACTER_WALK) {
		err = current_scene->walkMask->getNextPoint(c->coord, c->lastUpdate);
		switch(err) {
		case CHARACTER_STILL_WALKING:
			{
				if(lastAngle != c->coord.angle) {
					SetCurrentCharacterAnimation(c, false, true);
				}
				break;
			}
		case CHARACTER_ENDPOINT_REACHED:
			{
				stopCharacterMovement();
				dout << "movement ended successfully in: " << current_time << endl;
				break;
			}
		case CHARACTER_PATH_BLOCKED:
			{
				stopCharacterMovement();
				dout << "movement blocked- can't walk further in: " << current_time << endl;
				break;
			}
		default:
			{
				dout << "ERROR- WalkMask::getNextPoint returned invalid value: " << err << endl;
				exit(1);
			}
			
		}
	}
	else if (c->state == CHARACTER_ANIM) {
	}
	else if(c->state == CHARACTER_STAND) {
	}
	c->lastUpdate = current_time;
}


unsigned SetCharacterSpeed() {
	map<int, CharacterAnimation*>::iterator i = main_character->anims.find(main_character->state);
	if(i == main_character->anims.end()) {
		dout << "ERROR- Want to set speed for unknown character animation: " << main_character->state << endl;
		exit(1);
	}
	CharacterAnimation* anim = i->second;
	main_character->current_speed = anim->moveSpeed;
	return 0;
}

int stopCharacterMovement() {
	setCharacterState(CHARACTER_STAND);
	SetCurrentCharacterAnimation(main_character);
	return 0;
}

int startCharacterMovement(int new_x, int new_y) {
	// Returns:
	// <0 : Failure
	// 0 : Character is already on spot
	// >0 : Ok, character moving

	// Checks whenever character is on walkmask.
	// If not, character position is set to closest point to the mask.
	dout << " character will walk to point: " << new_x << "," << new_y <<  endl;

	int ret = 0;
	// Ok, lets start new movement.
	Character *c = main_character;
	if((c->coord.x != new_x) || (c->coord.y != new_y)) {

		stopTime();

		setCharacterState(CHARACTER_WALK);
		current_scene->walkMask->setSpeed(c->current_speed);
		int err = current_scene->walkMask->startNewPath(c->coord, new_x, new_y, current_time);
		SetCurrentCharacterAnimation(c); 

		resumeTime();
		ret = 1;
		dout << "startNewPath returned: " << err << endl;
	} else {
		dout << "Character is already in place" << endl;
	}

	dout << "movement started in: " << current_time << endl;
	main_character->lastUpdate = current_time;
	return ret;
}
