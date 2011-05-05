#include "game.h"

Hotspot* IsOnHotspot(int x, int y) {
	Hotspot* h;
	int relx, rely;
	if(current_scene->zbuflist.empty())
		return NULL;
	list<Hotspot*>::iterator i = current_scene->zbuflist.end();
	do{
		i--;
		h = *i;
		if((x >= h->x) && (y >= h->y)) {
			if(((x - h->x) < h->w) && ((y - h->y) < h->h)) {
				relx = x - h->x;
				rely = y - h->y;
				if(h->mask[relx + h->w * rely] > 0) {
					return h;
				}
			}
		}
	}while(i != current_scene->zbuflist.begin());
	return NULL;
}


unsigned ResortHotspots(Scene* s) {
	list<Hotspot*>::iterator i;
	HMI hmi = s->hotspots.begin();
	Hotspot* zh;
	Hotspot* sh;
	s->zbuflist.clear();
	while(hmi != s->hotspots.end()) {
		sh = hmi->second;
		if(sh->enabled) {
			i = s->zbuflist.begin();
			while(i != s->zbuflist.end()) {
				zh = *i;
				if((zh->depth) <= sh->depth) {
					s->zbuflist.insert(i, sh);
					break;
				}
				i++;
			}
			if(i == s->zbuflist.end()) {
				s->zbuflist.push_back(sh);
			}
		}
		hmi++;
	}
	
	return 0;
}

Scene* LoadScene(FILE* fin) {
	if(!IsNextString(fin, "scene")) {
		dout << " Loading scene failed- no 'scene' tag found" << endl;
		return NULL;
	}
	int id = LoadInt(fin);
	string gfx_file = LoadString(fin);
	string mask_file = LoadString(fin);
	string soundFile = LoadString(fin);
	convertPathInPlace(soundFile);
	int hotspot_id;
	Scene* s = new Scene;
	s->id = id;
	s->gfxFile = gfx_file;
	s->maskFile = mask_file;
	s->soundFile = soundFile;
	s->gfx = NULL;
	s->sound = NULL;

	if(IsNextString(fin, "hotspots")) {
		while(!IsNextString(fin, "end_hotspots")) {
			if(feof(fin)) {
				dout << " Loading scene failed- no 'end_hotspots' tag found" << endl;
				delete s;
				return NULL;
			}			
			hotspot_id = LoadInt(fin);
			if(s->hotspots.find(hotspot_id) == s->hotspots.end()) {
				Hotspot* h;
				HMI hmi = all_hotspots.find(hotspot_id);
				if(hmi == all_hotspots.end()) {
					dout << "ERROR- Unknown hotspot: " << hotspot_id << " defined in scene: " << s->id << endl;
					exit(1);
				} else {
					h = hmi->second;
					s->hotspots.insert(make_pair<unsigned, Hotspot*>(hotspot_id, h));
					dout << " Hotspot with id: " << hotspot_id << " added to scene: " << s->id << endl;
				}
			}
			else {
				dout << " Warning scene: " << s->id << " already contains hotspot with id: " << hotspot_id << endl;
			}

		}
	}
	if(!IsNextString(fin, "end_scene")) {
		delete s;
		dout << "ERROR- Loading scene failed- no 'end_scene' tag found" << endl;
		exit(1);
//		return NULL;
	}
	ResortHotspots(s);
	s->walkMask = new WalkMask(s->maskFile);
	return s;
}
void  LoadAllScenes(SceneMap& m) {
	dout << "--- LoadAllScenes" << endl;
	m.clear();
	
	vector<string> fileNames;
	loadListFile("scenes.lst", fileNames);
	
	FILE* fin;
	char str[256];
	Scene* s;
	for(int currFile = 0; currFile < fileNames.size(); currFile++) {
		strcpy(str, fileNames[currFile].c_str());
		fin = fopen(str, "r");
		if(!fin) {
			dout << "ERROR- 'scenes.lst' refers to invalid file: " << str << endl;
			exit(1);
		}
		dout << " Processing file: " << str << endl;
		while((s = LoadScene(fin))) {
			if(m.find(s->id) != m.end()) {
				dout << "ERROR- scene with id: " << s->id << "already exists. File: " << str << endl;
				exit(1);
			} 
			else {
				m.insert(make_pair<unsigned, Scene*>(s->id, s));
				dout << " Scene with id: " << s->id << " loaded" << endl;
			}
		}
		fclose(fin);
	}
	dout << "--- End LoadAllScenes" << endl << endl;

}
