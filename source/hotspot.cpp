#include "game.h"
#include "structs.h"

unsigned SetupHotspotScripts(HotspotMap& m, ScriptMap& sm) {
	HMI i = m.begin();
	SMI si = sm.begin();
	Script* s;
	Hotspot* h;
	while(i != m.end()) {
		h = i->second;
		h->scripts.clear();
		i++;
	}
	while(si != sm.end()) {
		s = si->second;
		if(s->type == SCRIPT_USE_ITEM) {
			i = m.find(s->param2);
			if(i == m.end()) {
				dout << "ERROR- unknown hotspot: " << s->param2 << " is reffered in script: " << s->id << endl;
				exit(1);
			}
			else {
				h = i->second;
				h->scripts.insert(make_pair<int, unsigned>(s->param1, s->id));
				dout << " script: " << s->id << " added to hotspot: " << h->id << "(" << h->description << ")" << endl;
			}
		}
		else {
			dout << " script: " << s->id << " is not 'use-item' script" << endl;
		}
		si++;
	}
	return 0;
}
unsigned SetHotspotAnim(Hotspot* h, int anim_id) {
	map<int, HotspotAnim*>::iterator i = h->anims.find(anim_id);
	if(i == h->anims.end()) {
		dout << "Could not set hotspot: " << h->id << "animation to: " << anim_id << endl;
		return 1;
	}
	HotspotAnim* a = i->second;
	h->anim_id = anim_id;
	h->anim = a->anim;
	resetAnimation(h->anim);
	h->description = a->description;
	h->h = a->h;
	h->mask = a->mask;
	h->w = a->w;
	h->x = a->x;
	h->y = a->y;
	h->sound = a->sound;
	h->restartSoundOnAnimLoop = a->restartSoundOnAnimLoop;
	return 0;
}

unsigned LoadHotspotData(Hotspot* h) {
	map<int, HotspotAnim*>::iterator i = h->anims.begin();
	while(i != h->anims.end()) {
		HotspotAnim* a = i->second;
		a->anim = LoadAnimation(a->gfxName, true, false);
		a->mask = loadMask(a->maskFile, a->w, a->h);
		convertPathInPlace(a->soundFile);
		a->sound = loadSoundCollection(a->soundFile, SOUND_TYPE_HOTSPOT);
		if(!a->anim) {
			dout << "ERROR- Could not load hotspot: " << h->id << " animation: " << a->gfxName << " gfx" << endl;
			exit(1);
		}
		if(!a->mask) {
			dout << "ERROR- Could not load hotspot: " << h->id << " animation mask: " << a->maskFile << endl;
			exit(1);
		}
		if(!a->sound) {
			dout << "WARNING- Could not load hotspot: " << h->id << " sound: " << a->soundFile << endl;
		}
		if(a->id == h->anim_id) {
			SetHotspotAnim(h, h->anim_id);
		}
		i++;
	}
	return 0;
}


Hotspot* LoadHotspot(FILE* fin) {
	if(!IsNextString(fin, "hotspot")) {
		dout << " Loading hotspot failed- no 'hotspot' tag found" << endl;
		return NULL;
	}
	int id = LoadInt(fin);
	int depth = LoadInt(fin);
	int enabled = LoadInt(fin);
	int first_anim_id = LoadInt(fin);
	int enterSceneMode = LoadInt(fin);
	int enableMode = LoadInt(fin);

	// Some little checks
	if(enterSceneMode < 0 || enterSceneMode >= HOTSPOT_SHOW_MODE_INVALID) {
		dout << "ERROR- Invalid hotspot enter scene diplay mode: " << enterSceneMode << endl;
		exit(1);
	}
	if(enableMode < 0 || enableMode >= HOTSPOT_SHOW_MODE_INVALID) {
		dout << "ERROR- Invalid hotspot enable diplay mode: " << enableMode << endl;
		exit(1);
	}

	Hotspot* h = new Hotspot;
	h->depth = depth;
	h->enabled = (enabled != 0);
	h->id = id;
	h->anim_id = first_anim_id;
	h->anim = NULL;
	h->x = 0;
	h->y = 0;
	h->h = 0;
	h->w = 0;
	h->description = "";
	h->mask = NULL;
	h->showModeEnterScene = enterSceneMode;
	h->showModeEnableHotspot = enableMode;
	dout << " Loading hotspot animations" << endl;
	while(IsNextString(fin, "hotspot_animation")) {
		HotspotAnim* a = new HotspotAnim;
		a->id = LoadInt(fin);
		a->x = LoadInt(fin);
		a->y = LoadInt(fin);
		a->restartSoundOnAnimLoop = (LoadInt(fin) > 0);
		a->gfxName = LoadString(fin);
		a->maskFile = LoadString(fin);
		a->soundFile = LoadString(fin);
		a->description = LoadString(fin, true);
		if(!IsNextString(fin, "end_hotspot_animation")) {
			delete h;
			dout << "ERROR- Loading hotspot failed- no 'end_hotspot_animation' tag found" << endl;
			exit(1);
		}
		if(h->anims.find(a->id) == h->anims.end()) {
			h->anims.insert(make_pair<unsigned, HotspotAnim*>(a->id, a));
		}
		else {
			dout << " ERROR- hotspot with id: " << h->id << " already has animation with id: " << a->id << endl;
			exit(1);
		}
	}
	if(!IsNextString(fin, "end_hotspot")) {
		dout << "ERROR- Loading hotspot failed- no 'end_hotspot' tag found" << endl;
		exit(1);
	}
	LoadHotspotData(h);
	return h;
}
void LoadAllHotspots(HotspotMap& m) {
	dout << "--- LoadAllHotspots" << endl;
	m.clear();

	vector<string> fileNames;
	loadListFile("hotspots.lst", fileNames);
	
	FILE* fin;
	char str[256];
	Hotspot* h;
	for(int currFile = 0; currFile < fileNames.size(); currFile++) {
		strcpy(str, fileNames[currFile].c_str());
		fin = fopen(str, "r");
		if(!fin) {
			dout << "ERROR: 'hotspots.lst' refers to invalid file: " << str << endl;
			exit(1);
		}
		dout << " Processing file: " << str << endl;
		while((h = LoadHotspot(fin))) {
			if(m.find(h->id) != m.end()) {
				dout << "ERROR: Hotspot with id: " << h->id << "already exists. File: " << str << endl;
				exit(1);
			}
			m.insert(make_pair<unsigned, Hotspot*>(h->id, h));
			dout << " Hotspot with id: " << h->id << " loaded" << endl;
		}
		fclose(fin);
	}
	SetupHotspotScripts(m, all_scripts);
	dout << "--- End LoadAllHotspots" << endl << endl;

}

int freeHotspotAnimData(Hotspot *h) {
	dout << "Free animation data for hotspot: " << h->id << endl;
	map<int, HotspotAnim*>::iterator animIter = h->anims.begin();
	while(animIter != h->anims.end()) {
		HotspotAnim *anim = animIter->second;
		freeAnimationData(anim->anim);
		animIter++;
	}
				
	return 0;
}

int loadHotspotAnimData(Hotspot *h) {
	dout << "Load animation data for hotspot: " << h->id << endl;
	map<int, HotspotAnim*>::iterator animIter = h->anims.begin();
	while(animIter != h->anims.end()) {
		HotspotAnim *anim = animIter->second;
		loadAnimationData(anim->anim);
		animIter++;
	}
	return 0;
}

