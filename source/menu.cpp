#include "game.h"
#include "gamestrings.h"

#ifdef __GNUC__
#include <sys/stat.h>
#else
#include <sys\stat.h>
#endif

struct MenuItem {
	int id;
	string text;
	bool enabled;

	MenuItem(string itemText, bool itemState, int itemId) {
		text = itemText;
		enabled = itemState;
		id = itemId;
	}

};



static const int MAX_MENU_ITEMS = 20;

static SDL_Surface *background = NULL;
static Animation *mouseCursor = NULL;
static vector<string> creditsText;
static int creditTimeSpeed;
static int creditScrollSpeed;

Mix_Music *uiMusic = NULL;

int doMenu(vector<MenuItem> items, Animation *mouseCursor, SDL_Surface *background, string title = " ");
int getString(string &text, Animation *mouseCursor = NULL, SDL_Surface *background = NULL, string title = "");
int selectGameSlot(Animation *mouseCursor, SDL_Surface *background, bool saveMenu = true);

int showScrollingText(vector<string> &text, SDL_Surface *background, int scrollSpeed, int timeInterval, int count = 1000);

int doGameMenu(int &param, bool resumeGameItem, int defaultAction) {

	/*
	string bla = "bla bla";
	getString(bla, NULL, NULL, "Hello");
	return MENU_EXIT;
	*/

	static bool firstTime = true;

	int wasMouseOn = SDL_ShowCursor(SDL_QUERY);
	if(!mouseCursor) {
		SDL_ShowCursor(SDL_DISABLE);
	}
	SDL_WarpMouse(screen->w / 2, screen->h / 2);

	int oldMusicVolume = 64;

	if(firstTime) {
		FILE *fin = fopen("ui.lst", "r");
		if(!fin) {
			dout << "ERROR- Could not open ui.lst file" << endl;
			exit(1);
		}
		string backgroundStr;
		string mouseStr;

		// Read game_menu tag from ui.lst file

		if(!IsNextString(fin, "game_menu")) {
			dout << "ERROR- ui.lst does not have game_menu tag" << endl;
			exit(1);
		}
		backgroundStr = LoadString(fin);
		mouseStr = LoadString(fin);

		if(!IsNextString(fin, "end_game_menu")) {
			dout << "ERROR- ui.lst does not have end_game_menu tag" << endl;
			exit(1);
		}

		if(backgroundStr.length() > 0) {
			background = loadImage(backgroundStr);
			if(!background) {
				dout << "ERROR- Could not load game menu background image (ui.lst): " << backgroundStr << endl;
				exit(1);
			}
		} else {
			dout << "Background string empty, so no background will be used" << endl;
		}

		if(mouseStr.length() > 0) {
			mouseCursor = LoadAnimation(mouseStr);
			if(!mouseCursor) {
				dout << "ERROR- failed to load game menu mouse cursor animation (ui.lst): " << mouseStr << endl;
				exit(1);
			}
		} else {
			dout << "Mouse cursor string empty, so no mouse cursor is loaded" << endl;
		}

		// Read game_credits tag from ui.lst file
		if(!IsNextString(fin, "game_credits")) {
			dout << "ERROR- ui.lst does not have game_credits tag" << endl;
			exit(1);
		}

		creditTimeSpeed = LoadInt(fin);
		creditScrollSpeed = LoadInt(fin);

		while(!IsNextString(fin, "end_game_credits")) {
			if(feof(fin)) {
				dout << "ERROR- ui.lst does not have end_game_credits tag" << endl;
				exit(1);
			}
			string text = LoadString(fin, true);
			creditsText.push_back(text);
		}

		// Read startup_movies tag from ui.lst file
		vector<string> startupMovies;
		if(!IsNextString(fin, "startup_movies")) {
			dout << "ERROR- ui.lst does not have startup_movies tag" << endl;
			exit(1);
		}
		while(!IsNextString(fin, "end_startup_movies")) {
			if(feof(fin)) {
				dout << "ERROR- ui.lst does not have end_startup_movies tag" << endl;
				exit(1);
			}
			string movieFile = LoadString(fin);
			startupMovies.push_back(movieFile);
		}

		// Read save_game_dir tag from ui.lst file
		if(!IsNextString(fin, "save_game_dir")) {
			dout << "ERROR- ui.lst does not have save_game_dir tag" << endl;
			exit(1);
		}
		saveGameDir = LoadString(fin);
#ifdef unix
		saveGameDir = getenv("HOME") + string(SEPERATOR) + saveGameDir;
		dout << "  saveGameDir = " << saveGameDir << endl;
		mkdir(saveGameDir.c_str(), 0755);
#endif
		if(!IsNextString(fin, "end_save_game_dir")) {
				dout << "ERROR- ui.lst does not have end_save_game_dir tag" << endl;
				exit(1);
		}

		// Read menu music (if any) tag from ui.lst file
		if(IsNextString(fin, "ui_music")) {
			string uiMusicFile = LoadString(fin);
			if(!IsNextString(fin, "end_ui_music")) {
					dout << "ERROR- ui.lst does not have end_ui_music tag" << endl;
					exit(1);
			}

			if(!(uiMusic = Mix_LoadMUS(uiMusicFile.c_str()))) {
				dout << "ERROR- failed to load music file: " << uiMusicFile << endl;
				exit(1);
			}
		}

		fclose(fin);

		firstTime = false;

		for(int a = 0; a < startupMovies.size(); a++) {
			playVideo(startupMovies[a]);
		}
	}

	if(defaultAction >= 0) {
		SDL_ShowCursor(wasMouseOn);
		SDL_WarpMouse(current_mouse_x, current_mouse_y);
		return defaultAction;
	}
	
	vector<MenuItem> items;
	items.push_back(MenuItem(STRING_MENU_NEW_GAME, true, MENU_NEW_GAME));
	items.push_back(MenuItem(STRING_MENU_SAVE_GAME, resumeGameItem, MENU_SAVE_GAME));
	items.push_back(MenuItem(STRING_MENU_LOAD_GAME, true, MENU_LOAD_GAME));
	items.push_back(MenuItem(STRING_MENU_RESUME_GAME, resumeGameItem, MENU_RESUME_GAME));
	items.push_back(MenuItem(STRING_MENU_CREDITS, true, MENU_CREDITS));

	items.push_back(MenuItem(STRING_MENU_EXIT_GAME, true, MENU_EXIT));

	int ret;
	param = 0;

	if(uiMusic) {
		oldMusicVolume = Mix_VolumeMusic(64);
		Mix_PlayMusic(uiMusic, -1);
	}

	while(1) {
		ret = doMenu(items, mouseCursor, background, STRING_MENU_NAME_MAIN);
		if(ret == MENU_EXIT) {
			vector<MenuItem> exititems;
			exititems.push_back(MenuItem(STRING_MENU_EXIT_YES, true, 1));
			exititems.push_back(MenuItem(STRING_MENU_EXIT_NO, true, 0));
			int shouldQuitGame = doMenu(exititems, mouseCursor, background, STRING_MENU_REALLY_EXIT);
			if((shouldQuitGame == -2) || (shouldQuitGame == 1)) {
				ret = MENU_EXIT;				
			} else {
				continue;
			}
			

		} else if(ret == -1) {
			// ESC pressed
			if(resumeGameItem) {
				ret = MENU_RESUME_GAME;
			} else {
				ret = MENU_EXIT;
			}
		} else if(ret == -2) {
			// Pressed close button
			ret = MENU_EXIT;
		} else if(ret == MENU_LOAD_GAME) {
			int loadSlot = selectGameSlot(mouseCursor, background, false);
			if(loadSlot == -1) {
				continue;
			} else if (loadSlot == -2) {
				ret = MENU_EXIT;
			} else {
				param = loadSlot;
			}
		} else if(ret == MENU_SAVE_GAME) {
			int saveSlot = selectGameSlot(mouseCursor, background, true);
			if(saveSlot == -1) {
				continue;
			} else if (saveSlot == -2) {
				ret = MENU_EXIT;
			} else {

				char gameFileName[256];
				static char buf[1024];
				memset(buf, 0, 1024);
				
				if(saveGameDir.length() > 0) {
					sprintf(gameFileName, "%s" SEPERATOR "%.2i_name.dat", saveGameDir.c_str(), saveSlot);
				} else {
					sprintf(gameFileName, "%.2i_name.dat", saveSlot);
				}

				string saveGameName = "???";

				FILE *fin = fopen(gameFileName, "rb");
				if(!fin) {
					saveGameName = STRING_EMPTY_GAMESLOT_NAME;
				} else {
					unsigned n = fread(buf, 1024, 1, fin);
					fclose(fin);
					saveGameName = buf;
				}

				int nameRet = getString(saveGameName, mouseCursor, background, STRING_SAVEGAME_NAME);
				if(nameRet == -2) {
					ret = MENU_EXIT;
				} else if(nameRet == -1) {
					continue;
				} else {
					// Remove trailing whitespaces
					while((saveGameName.length()) > 0 && (saveGameName[saveGameName.length() - 1] == ' ')) {
						saveGameName.resize(saveGameName.length() - 1);
					}

					FILE *fout = fopen(gameFileName, "wb");
					if(!fout) {
						dout << "ERROR- Could not open file for writing: " << gameFileName << endl;
						exit(1);
					}
					fwrite(saveGameName.c_str(), saveGameName.length(), 1, fout);
					fclose(fout);
				}
				param = saveSlot;
			}
		} else if(ret == MENU_CREDITS) {
			showScrollingText(creditsText, background, creditScrollSpeed, creditTimeSpeed, 1);
			continue;
		}
		break;
	}

	if(Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}

	if(uiMusic) {
		Mix_VolumeMusic(oldMusicVolume);
	}

	SDL_ShowCursor(wasMouseOn);
	SDL_WarpMouse(current_mouse_x, current_mouse_y);
	return ret;	
}


int doMenu(vector<MenuItem> items, Animation *mouseCursor, SDL_Surface *background, string title) {
	bool shouldExit = false;
	int currentY;
	int currentX;
	static int mouseX = 0;
	static int mouseY = 0;
	int currItem = -1;

	int a;
	Font_Struct *font;
	SDL_Rect itemRectangle[MAX_MENU_ITEMS];
	SDL_Rect r;
	int ret = -1;
	SDL_Event e;

	int normalItemSize = 0;

	dialogNormalText->strRect("I", &r);
	normalItemSize = r.h + 2;

	SDL_GetMouseState(&mouseX, &mouseY);

	while(!shouldExit) {

		currentY = (screen->h - normalItemSize * (items.size() + 1)) / 2 ;

		if(background) {
			myBlitSurface(background, NULL, screen, NULL);
		} else {
			myFillRect(screen, NULL, 0);
		}

		font = dialogSelectedText;
		font->strRect(title.c_str(), &r);
		currentX = (screen->w - r.w) / 2;
		font->writeStr(screen, title.c_str(), currentX, currentY, NULL);
		currentY += r.h + normalItemSize;
		
		for(a = 0; a < items.size(); a++) {
			MenuItem &item = items[a];
			if(item.enabled) {
				font = dialogNormalText;
				if(a == currItem) {
					font = dialogSelectedText;
				}
			} else {
				font = dialogVisitedText;
			}

			font->strRect(item.text.c_str(), &r);
			currentX = (screen->w - r.w) / 2;
			font->writeStr(screen, item.text.c_str(), currentX, currentY, NULL);

			itemRectangle[a] = r;
			itemRectangle[a].x = currentX;
			itemRectangle[a].y = currentY;
			currentY += r.h + 2;
		}

		if(mouseCursor) {
			DrawAnimation(screen, mouseCursor, mouseX, mouseY);
		} else {
			r.x = mouseX - 4;
			r.y = mouseY;
			r.w = 9;
			r.h = 1;
			myFillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
			r.x = mouseX;
			r.y = mouseY - 4;
			r.w = 1;
			r.h = 9;
			myFillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
		}

		SDL_GL_SwapBuffers();
		
		while(SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_QUIT:
				{
					ret = -2;
					shouldExit = true;
					break;
				}
			case SDL_MOUSEMOTION:
				{
					int x = e.motion.x;
					int y = e.motion.y;
					currItem = -1;
					for(int a = 0; a < items.size(); a++) {
						SDL_Rect &r = itemRectangle[a];
						if((r.y < y) && (r.y + r.h > y))  {
							currItem = a;
							break;
						}
					}
					mouseX = x;
					mouseY = y;
					break;
				}
			case SDL_MOUSEBUTTONUP:
				{
					if(currItem >= 0) {
						MenuItem &item = items[currItem];
						if(item.enabled) {
							ret = item.id;
							shouldExit = true;
						}
					}
					break;
				}
			case SDL_KEYUP:
				{
					switch(e.key.keysym.sym) {
					case SDLK_ESCAPE:
						{
							ret = -1;
							shouldExit = true;
							break;
						}
					}
				}
			}
		}
	}
	return ret;
}

int selectGameSlot(Animation *mouseCursor, SDL_Surface *background, bool saveMenu) {
	int a;
	int ret = 0;
	vector<MenuItem> items;
	char fileName[256];
	string slotName;
	bool enabled;
	string title;
//	char slotNumberBuf[10];

	for(a = 0; a < 10; a++) {
		static char buf[1024];
		memset(buf, 0, 1024);

		enabled = true;

		if(saveGameDir.length() > 0) {
			sprintf(fileName, "%s" SEPERATOR "%.2i_name.dat", saveGameDir.c_str(), a);
		} else {
			sprintf(fileName, "%.2i_name.dat", a);
		}
//		sprintf(slotNumberBuf, "%.2i.  ", a + 1);
//		slotName = slotNumberBuf;

		FILE *fin = fopen(fileName, "rb");
		if(!fin) {
			slotName = STRING_SLOT_EMPTY;
			if(!saveMenu) {
				enabled = false;
			}
		} else {
			unsigned n = fread(buf, 1024, 1, fin);
			fclose(fin);
			slotName = buf;
			if(slotName.length() == 0) {
				slotName = STRING_NONAMED_SAVEGAME;
			}
		}

		items.push_back(MenuItem(slotName, enabled, a));
	}
	items.push_back(MenuItem("  ", false, 0));
	items.push_back(MenuItem(STRING_MENU_CANCEL, true, -1));

	if(saveMenu) {
		title = STRING_SELECT_SAVEGAME;
	} else {
		title = STRING_SELECT_LOADGAME;
	}

	ret = doMenu(items, mouseCursor, background, title);

	if((saveMenu) && (ret >= 0)){
		MenuItem &item = items[ret];
		if(item.text != STRING_SLOT_EMPTY) {
			vector<MenuItem> overwriteitems;
			char buf[256];
			sprintf(buf, STRING_MENU_OVERWRITE_GAME, item.text.c_str());
			overwriteitems.push_back(MenuItem(STRING_MENU_OVERWRITE_YES, true, 1));
			overwriteitems.push_back(MenuItem(STRING_MENU_OVERWRITE_NO, true, 0));
			int shouldOverwrite = doMenu(overwriteitems, mouseCursor, background, buf);
			if(shouldOverwrite == -2) {
				ret = -2;
			} else if(shouldOverwrite != 1) {
				ret = -1;
			}
		}
	}
	return ret;
}

int getString(string &text, Animation *mouseCursor, SDL_Surface *background, string title) {
	SDL_Event e;
	int currPos = text.length();
	bool shouldExit = false;
	Font_Struct *title_font = dialogSelectedText;
	Font_Struct *text_font = dialogNormalText;
	Font_Struct *cancelFont = NULL;

	int mouseX = 0;
	int mouseY = 0;

	SDL_Rect cancelRect;
	bool cancelSelected = false;
	SDL_Rect r;
	int x, y;
	int ret = 0;

	string tmptext;

	SDL_GetMouseState(&mouseX, &mouseY);

	while(!shouldExit) {

		if(background) {
			myBlitSurface(background, NULL, screen, NULL);
		} else {
			myFillRect(screen, NULL, 0);
		}

		title_font->strRect(title.c_str(), &r);
		x = (screen->w - r.w) / 2;
		y = ((screen->h - r.h) / 2) - ((r.h + 2) * 2);
		title_font->writeStr(screen, title.c_str(), x, y, &r);

		tmptext = text + "#";
		text_font->strRect(tmptext.c_str(), &r);
		x = (screen->w - r.w) / 2;
		y = (screen->h - r.h) / 2;
		text_font->writeStr(screen, tmptext.c_str(), x, y, &r);

		if(cancelSelected) {
			cancelFont = title_font;
		} else {
			cancelFont = text_font;
		}

		cancelFont->strRect(STRING_MENU_CANCEL_SAVEGAME, &r);
		x = (screen->w - r.w) / 2;
		y = ((screen->h - r.h) / 2) + ((r.h + 2) * 2);
		cancelFont->writeStr(screen, STRING_MENU_CANCEL_SAVEGAME, x, y, &cancelRect);
		

		if(mouseCursor) {
			DrawAnimation(screen, mouseCursor, mouseX, mouseY);
		} else {
			r.x = mouseX - 4;
			r.y = mouseY;
			r.w = 9;
			r.h = 1;
			myFillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
			r.x = mouseX;
			r.y = mouseY - 4;
			r.w = 1;
			r.h = 9;
			myFillRect(screen, &r, SDL_MapRGB(screen->format, 255, 0, 0));
		}

		SDL_GL_SwapBuffers();
		
		while(SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_QUIT:
				{
					ret = -2;
					shouldExit = true;
					break;
				}
			case SDL_MOUSEMOTION:
				{
					mouseX = e.motion.x;
					mouseY = e.motion.y;
					cancelSelected = false;
					if((mouseY >= cancelRect.y) && (mouseY <= cancelRect.y + cancelRect.h)) {
						cancelSelected = true;
					}
					break;
				}
			case SDL_MOUSEBUTTONUP:
				{
					if(cancelSelected) {
						ret = -1;
						shouldExit = true;
					}
					break;
				}
			case SDL_KEYUP:
				{
					switch(e.key.keysym.sym) {
					case SDLK_ESCAPE:
						{
							ret = -1;
							shouldExit = true;
							break;
						}
					case SDLK_BACKSPACE:
						{
							if(text.length() > 0) {
								text.resize(text.length() - 1);
							}
							break;
						}
					case SDLK_RETURN:
						{
							shouldExit = true;
							break;
						}
					default:
						{
							int key = e.key.keysym.sym;
							if((key >= SDLK_a) && (key <= SDLK_z)) {
								char c;
								if(e.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
									c = 'A' + (key - SDLK_a);
								} else {
									c = 'a' + (key - SDLK_a);
								}
								text += c;
							} else if((key >= SDLK_0) && (key <= SDLK_9)) {
								char c = '0' + (key - SDLK_0);
								text += c;
							} else if(key == SDLK_SPACE) {
								text += ' ';
							}
							break;
						}
					}
					break;
				}
			}
		}
	}
	return ret;
}


int showScrollingText(vector<string> &text, SDL_Surface *background, int scrollSpeed, int timeInterval, int count) {
	bool shouldExit = false;
	SDL_Event e;
	Font_Struct *font = dialogNormalText;

	int a;
	int currentY = 0;
	int currentX = 0;
	int yOfs = screen->h;
	SDL_Rect rect;
	unsigned startTime = SDL_GetTicks();
	unsigned lastTime = startTime;
	unsigned currentTime = startTime;

	int totalHeight = 0;

	int timesShown = 0;

	for(a = 0; a < text.size(); a++) {
		font->strRect(text[a].c_str(), &rect);
		totalHeight += rect.h;
	}

	while(!shouldExit && timesShown < count) {

		currentTime = SDL_GetTicks();
		if((currentTime - lastTime) >=  timeInterval) {
			yOfs-=scrollSpeed;
			if(yOfs < -totalHeight) {
				yOfs = screen->h;
				timesShown++;
			}
			lastTime = currentTime;
		}



		currentY = 0;		

		if(background) {
			myBlitSurface(background, NULL, screen, NULL);
		} else {
			myFillRect(screen, NULL, 0);
		}
		for(a = 0; a < text.size(); a++) {
			font->strRect(text[a].c_str(), &rect);
			currentX = (screen->w - rect.w) / 2;
			// Write only visible strings
			if(!(currentY + yOfs <= 0)) {
				font->writeStr(screen, text[a].c_str(), currentX, currentY + yOfs, &rect);
			}
			currentY += rect.h;
		}
		
		SDL_GL_SwapBuffers();

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_QUIT:
				{
					shouldExit = true;
					break;
				}
			case SDL_MOUSEBUTTONUP:
				{
					shouldExit = true;
					break;
				}
			case SDL_KEYUP:
				{
					shouldExit = true;
					break;
				}
			}
		}
	}
	return 0;
}

int showGameCredits() {
	Mix_Pause(-1);
	showScrollingText(creditsText, background, creditScrollSpeed, creditTimeSpeed, 1);
	return 0;
}
