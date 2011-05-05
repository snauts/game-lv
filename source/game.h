#ifndef __GAME_H__
#define __GAME_H__


#pragma warning(disable : 4786)


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef unix
	#include <errno.h>
	#include <unistd.h>
	#include <string.h>
#else
	#include <io.h>
#endif


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>	// Header File For The OpenGL32 Library
#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#endif

#include <SDL.h>
#include <SDL_mixer.h>
#include <smpeg.h>
#include <SDL_image.h>


#include <string>
#include <map>
#include <vector>

using namespace std;

#pragma warning(disable : 4786)

#include "music.h"
#include "sound.h"
#include "consts.h"
#include "debug.h"
#include "structs.h"
#include "load.h"
#include "anim.h"
#include "walkmask.h"
#include "state.h"
#include "character.h"
#include "script.h"
#include "scene.h"
#include "sysfonc.h"
#include "savegame.h"
#include "loadgame.h"
#include "fonts.h"
#include "video.h"
#include "fade.h"
#include "hotspot.h"
#include "inventory.h"
#include "defaultInventory.h"
#include "menu.h"
#include "spline.h"

// Global variables
extern SDL_Surface *screen;
extern volatile long current_time;
extern volatile bool isTimeTicking;
extern bool isSoundPossible;
extern MusicPlayer musicPlayer;
extern SoundPlayer soundPlayer;

extern unsigned current_game_state;
extern int state_var1;
extern int state_var2;

extern debug_out dout;

extern ScriptMap all_scripts;
extern HotspotMap all_hotspots;
extern SceneMap all_scenes;
extern InventoryItemMap all_inventory_items;
extern DialogMap all_dialogs;

extern Character *main_character;
extern Scene *current_scene;
extern InventoryItemMap inventory;

extern Dialog *currentDialog;
extern DialogQuestion *currentDialogQuestion;
extern DialogAnswer *currentDialogAnswer;

extern InventorySkin *currentInventorySkin;

extern int current_mouse_x;
extern int current_mouse_y;
extern int current_buttons;

extern int currentViewX;
extern int currentViewY;

extern state_machine<string, int, debug_out> game_states;

extern map<int, ScriptTrigger> triggeredScripts;

extern bool should_draw_text;
extern list<TextItem> onscreen_text;

extern FontMap allFonts;
extern Font_Struct *normalFont;
extern Font_Struct *debugFont;
extern Font_Struct *dialogNormalText;
extern Font_Struct *dialogVisitedText;
extern Font_Struct *dialogSelectedText;
extern Font_Struct *dialogQuestionText;
extern Font_Struct *dialogAnswerText;


// Shitie pie save/load paliek nemainiigi.
//extern SoundCollection *uiSound;
extern Mix_Music *uiMusic;
extern map<int, SoundCollection*> scriptSounds;

extern SDL_Surface *savedScreen;
extern GLuint savedScreenTexture;
extern int savedScreenTextureW;
extern int savedScreenTextureH;
extern float savedScreenTextureX;
extern float savedScreenTextureY;

extern bool shouldFade;
extern int fadeMethod;
extern int fadeSpeed;
extern int fadeProgress;

extern bool doCharacterAntialiasing;

extern string saveGameDir;

extern int maximalTimeSkip;
extern bool debugConsole;

// General game functions
// game.cpp
unsigned genericInit();
unsigned newGameInit();
unsigned ResumeGame();
unsigned DrawSceneNormal();
unsigned DrawSceneNormalDisabled();
unsigned DrawSceneDialog();
unsigned DrawSceneDialogDisabled();

bool drawFadeEffect(int method, int speed, int &progress, SDL_Surface *savedScreen);
unsigned resetGame();

unsigned enterGameState(unsigned gameState, int var1 = 0, int var2 = 0);

unsigned TimerCallback(unsigned interval);
void stopTime();
void resumeTime();

// dialog.cpp

unsigned startDialog(Dialog *d, int firstQuestionId = -1);
int parseSayString(const std::string &str, std::string &part1, std::string &part2);

int myBlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
int myBlitTexture(GLuint tex, float xofs, float yofs, SDL_Rect *dstrect, SDL_Rect *srcrect = NULL, Uint16 width = 0, Uint16 height = 0);
int myFillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color, float alpha = 1.0);

int nearestPow2(int x);

#endif

