#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <string>
#include <map>
#include <vector>
#include <list>

#include "walkmask.h"
#include "sound.h"
#include "consts.h"
#include "spline.h"

using namespace std;

struct Animation;
struct HotspotAnim;
struct Hotspot;
struct ScriptAction;
struct Script;

class MyException;

struct AnimationTransform {
	int type;
	int length;
	int currentX;
	int currentY;
	long startTime;
	long endTime; // should be startTime + length;
	long lastUpdate;
};

struct LinearAnimationTransform {
	AnimationTransform transform;

	int startX;
	int startY;
	int endX;
	int endY;
};

struct SplineAnimationTransform {
	AnimationTransform transform;

	vector <PointStruct> points; 
};

struct AnimationBlock {
	int xOfs;
	int yOfs;

	unsigned totalFrames;

	SDL_Surface **img;
	vector<string> imgFiles;

	unsigned speed;

	bool hasColorKey;
	bool hasAlpha;
	unsigned alpha;
	unsigned colorKey;

	AnimationBlock(); // In animation.cpp
	~AnimationBlock(); // In animation.cpp
};

struct Animation {
	bool isTransformSynchronised;
	AnimationTransform **transforms;
	AnimationTransform *currentTransform;
	int totalTransforms;
	int currentTransformIdx;
	unsigned currentTransformLoops;
	
	AnimationBlock **blocks;
	AnimationBlock *currentBlock;
	int totalBlocks;
	int currentBlockIdx;
	unsigned currentLoops;
	
	int lastUpdate;
	int currentFrame; // In currentBlock.


	bool useVideoMemory;
	bool isImageDataLoaded;


	Animation(); // In animation.cpp
	~Animation(); // In animation.cpp
	int getCurrentFrameWidth() {
		return currentBlock->img[currentFrame]->w;
	}
	int getCurrentFrameHeight() {
		return currentBlock->img[currentFrame]->h;
	}
};

struct HotspotAnim {
	int id;
	int x;
	int y;
	string gfxName;
	string maskFile;
	string soundFile;
	string description;
	Animation *anim;
	unsigned char *mask;
	SoundCollection *sound;
	int w;
	int h;
	bool restartSoundOnAnimLoop;

	HotspotAnim() {
		id = -1;
		x = 0;
		y = 0;
		w = 0;
		h = 0;
		anim = NULL;
		mask = NULL;
		restartSoundOnAnimLoop = false;
	}

	~HotspotAnim() {
		if(anim) {
			delete anim;
		}
		if(mask) {
			delete mask;
		}
		if(sound) {
			delete sound;
		}
	}
};

struct Hotspot {
	int id;
	int anim_id;
	int x;
	int y;
	map<int, HotspotAnim*> anims;
	Animation* anim; // pashreizeejaa animaacija
	SoundCollection *sound; // pashreizeejas animaacijas skanja.
	bool restartSoundOnAnimLoop;
	int w; // maskas w
	int h; // maskas h
	unsigned char* mask;
	string description; // paraadaas kad pele uzbrauc uz maskas
	int depth; // prieksh z buffera
	bool enabled;
	multimap<int, int> scripts; // item id, script id
	int showModeEnterScene;
	int showModeEnableHotspot;
	
	Hotspot() {
		id = -1;
		anim_id = -1;
		anim = NULL;
		mask = NULL;
		enabled = false;
		sound = NULL;
		restartSoundOnAnimLoop = false;
		showModeEnterScene = HOTSPOT_RESET_ANIM_RESTART_SOUND;
		showModeEnableHotspot = HOTSPOT_RESET_ANIM_RESTART_SOUND;
	}

	~Hotspot() {
		map<int, HotspotAnim*>::iterator i = anims.begin();
		while(i != anims.end()) {
			delete i->second;
			i++;
		}
	}
};


struct ScriptAction {
	int action;
	int param1;
	int param2;
	int param3;
	int param4;
	string str;

	// If action suspends he can use this value for temporary
	// data.
	// defined in scripts.cpp
	static int tmpval[SCRIPT_ACTION_STATIC_DATA_SIZE];

	ScriptAction() {
		action = ACTION_DO_NOTHING;
		param1 = 0;
		param2 = 0;
		param3 = 0;
		param4 = 0;
	}
};


struct Script {
	int id;
	int execute_times;
	int type;
	int param1;
	int param2;
	vector<ScriptAction*> actions;
	~Script() {
		for(unsigned a = 0; a < actions.size(); a++)
			delete actions[a];
	}
};

// Typedefs

typedef map<int, Script*> ScriptMap;
typedef ScriptMap::iterator SMI;
typedef map<int, Hotspot*> HotspotMap;
typedef HotspotMap::iterator HMI;

struct ScriptTrigger {
	int triggeredTill;
	int scriptId;
	int triggerType;
};

struct Scene {
	int id;
	string gfxFile;
	string maskFile;
	string soundFile;
	
	HotspotMap hotspots;

	SoundCollection *sound;

	Animation* gfx;
	int w;
	int h;
	WalkMask *walkMask;
	list<Hotspot*> zbuflist;
	Scene() : walkMask(NULL), sound(NULL), gfx(NULL) {
	}
	~Scene() {
		if(sound) delete sound;
		if(gfx) delete gfx;
	}
};

typedef map<unsigned, Scene*> SceneMap;
typedef SceneMap::iterator SCMI;

struct CharOneWayAnim {
	vector<string> gfxFile;
	Animation **anim;
	CharOneWayAnim() {
		anim = NULL;
	}
	~CharOneWayAnim() {
		if(anim) {
			for(unsigned a = 0; a < gfxFile.size(); a++) {
				delete anim[a];
			}
			delete[] anim;
		}
	}
};


struct CharacterAnimation {
	int id;
	int directions;
	int animationsPerDirection;
	int moveSpeed;
	string soundFile;
	SoundCollection *sound;
	vector<CharOneWayAnim*> anims;

	CharacterAnimation() {
		sound = NULL;
	}
	~CharacterAnimation() {
		for(unsigned a = 0; a < anims.size(); a++) {
			delete anims[a];
		}
		if(sound) {
			delete sound;
		}
	}
};

typedef int (*isActionCompleteHandler)() ;

struct CharacterAnimationSet {
	int id;
	string characterName;
	map<int, int> mappedAnimationIds;
	map<int, CharacterAnimation*> anims;
	~CharacterAnimationSet() {
		map<int, CharacterAnimation*>::iterator i = anims.begin();
		while(i != anims.end()) {
			delete i->second;
			i++;
		}
	}
};


struct Character {

	CoordinatePoint coord;
	string characterName; // Character name or whatever.

	int state; // Walking, standing, idle...
	int prevState; // what was previous state
	long lastUpdate;

	int current_speed;

	bool visible;
	int currentAnimSize;
	Animation** current_anim;
	map<int, CharacterAnimation*> anims;
	map<int, int> mappedAnimationIds;
	map<int, CharacterAnimationSet*> animationSet;
	int currentAnimationSetId;

	Script* suspended_script;
	ScriptAction* suspended_action;
	isActionCompleteHandler isActionComplete;

	Character() {
		visible = true;
		currentAnimationSetId = -1;
		current_speed = 0;
		current_anim = NULL;
		state = CHARACTER_STAND;
		prevState = -1;
		lastUpdate = 0;
		suspended_script = NULL;
		suspended_action = NULL;
		isActionComplete = NULL;
	}
	~Character() {
		map<int, CharacterAnimationSet*>::iterator i = animationSet.begin();
		while(i != animationSet.end()) {
			delete i->second;
			i++;
		}
	}
};


struct InventoryItem {
	int id;
	string short_name;
	string long_name;
	string unselectedGfxFile;
	string selectedGfxFile; // Can be "", then unselected gfx is used.
	Animation *unselectedGfx;
	Animation *selectedGfx;
	multimap<int, int> scripts; // item id, script id. Used when item is used on item.
	InventoryItem() {
		unselectedGfx = NULL;
		selectedGfx = NULL;
	}
	~InventoryItem() {
		if(selectedGfx != unselectedGfx) {
			if(unselectedGfx) {
				delete unselectedGfx;
			}
			if(selectedGfx) {
				delete selectedGfx;
			}
		} else {
			if(unselectedGfx) {
				delete unselectedGfx;
			}
		}
	}
};

typedef map<int, InventoryItem*> InventoryItemMap;
typedef InventoryItemMap::iterator IIMI;

struct Font_Struct;
typedef map<int, Font_Struct*> FontMap;
typedef FontMap::iterator FMI;

struct TextItem {
	string text;
	int expireTime;
	Font_Struct *font;
	int fontId;
};


struct DialogAnswer {
	int id;
	bool enabled;
	vector<string> answerChoose;
	vector<string> answerReal;
	vector<string> soundFiles;
	int currentAnswerIdx;
	int answerSelectMode;
	int nextQuestionId;
	bool visited;
	int inScriptId; // >0 : script id, -1 : no script.
	int outScriptId; // >0 : script id, -1 : no script.

	DialogAnswer() {
		id = -1;
		currentAnswerIdx = -1;
		answerSelectMode = ANSWER_SELECT_FIRST;
		enabled = true;
		visited = false;
		nextQuestionId = -1;
		inScriptId = -1;
		outScriptId = -1;
	}
};

struct DialogQuestion {
	int id;
	vector<string> questions;
	vector<string> soundFiles;
	int currentQuestionIdx;
	int questionSelectMode;
	vector<DialogAnswer> answers;
	int inScriptId; // >0 : script id, -1 : no script.
	int outScriptId; // >0 : script id, -1 : no script.
	int enabledAnswers;
	int nextDialogQuestion;

	DialogQuestion() {
		id = -1;
		enabledAnswers = 0;
		currentQuestionIdx = -1;
		questionSelectMode = QUESTION_SELECT_FIRST;
		nextDialogQuestion = -1;
		inScriptId = -1;
		outScriptId = -1;
	}
};

struct Dialog {
	int id;
	int firstQuestionId;
	map<int, DialogQuestion> questions;
};


typedef map<unsigned, Dialog*> DialogMap;
typedef DialogMap::iterator DMI;
/*
class MyException {
public:
	string reason;
	MyException(string& str) {
		reason = str;
	}
	MyException(const char* str) {
		reason = (string)str;
	}
	MyException() {
		reason = "Unknown reason";
	}
	MyException(int line, char* file, string& reas) {
		char str[256];
		sprintf(str, "Error: %s, line: %d, reason: %s\n", file, line, reas.c_str());
		reason = (string)str;
	}
};
*/


#endif
