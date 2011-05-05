#ifndef __LOAD_H__
#define __LOAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "structs.h"

using namespace std;

#ifdef unix
#define SEPERATOR "/"
string convertPath(const string &str);
string &convertPathInPlace(string &str);
char *convertPathInPlace(char *str);
#else

#define SEPERATOR "\\"
#define convertPath(x) (x)
#define convertPathInPlace(x) (x)
#endif

// readfile.cpp
int loadSymbolsMap();
int LoadInt(FILE* fin);
string LoadString(FILE* fin, bool replaceSpecialSymbols = false);
bool ByPassString(FILE* fin, const string& str);
bool IsNextString(FILE* fin, const string& str, bool should_bypass = true);
void TrimWhitespaces(char* str);
int loadListFile(const char *fileName, vector<string> &fileNames);

// load.cpp 
SDL_Surface* loadImage(const string& img, bool has_color_key = false, int color_key = 0, 
					   bool has_alpha = false, int opacity = 100,
					   bool shouldConvertToDisplayFormat = true, bool useVideoMemory = true,
					   bool shouldCache = true, bool requiresTexture = true);
void freeImage(SDL_Surface *surf);
unsigned char* loadMask(const string& mask_file, int& w, int& h);

//inventory.cpp
InventoryItem* LoadInventoryItem(FILE* fin);
void LoadAllInventoryItems(InventoryItemMap& m);
unsigned LoadInventoryItemData(InventoryItem* i);
unsigned FreeInventoryItemData(InventoryItem* i);


// script.cpp
Script* LoadScript(FILE* fin);
void LoadAllScripts(ScriptMap& m);

// hotspot.cpp
unsigned SetHotspotAnim(Hotspot* h, int anim_id);
unsigned LoadHotspotData(Hotspot* h);
Hotspot* LoadHotspot(FILE* fin);
void LoadAllHotspots(HotspotMap& m);

// scene.cpp
Scene* LoadScene(FILE* fin);
void  LoadAllScenes(SceneMap& m);

// character.cpp
Character* LoadCharacter(const string& filename);

// dialog.cpp
void LoadAllDialogs(DialogMap &m);

struct ImageStruct {
	SDL_Surface *img;
	int refCount;
	std::string fileName;
	bool hasColorKey;
	int colorKey;
	bool hasAlpha;
	int alpha;
	bool shouldConvert;
	bool useVideoMemory;

	GLuint texture;
	float textureXCoord;
	float textureYCoord;

	ImageStruct() {
		texture = 0;
		refCount = 0;
		img = NULL;
	}
};

extern std::map<SDL_Surface*, ImageStruct*> loadedImages;

#endif
