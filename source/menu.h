#ifndef __MENU_H__
#define __MENU_H__

enum {
	MENU_NEW_GAME = 0,
	MENU_SAVE_GAME,
	MENU_LOAD_GAME,
	MENU_RESUME_GAME,
	MENU_CREDITS,
	MENU_EXIT
};


int doGameMenu(int &param, bool resumeGameItem = false, int defaultAction = -1);
int showGameCredits();

#endif
