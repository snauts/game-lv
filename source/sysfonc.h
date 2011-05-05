#ifndef __SYSFONC_H__
#define __SYSFONC_H__

#include "load.h"

// constantes
const unsigned char_count = 256;

const char lineFeed = 0x0A;
const char carriage = 0X0D;
const char spaceSymbol = 32;
const char endSymbol = 0;

// globals
void composeRects(SDL_Rect *rect1, SDL_Rect *rect2);
bool isStringEnd(char*);
bool isSpace(char*);
bool isLineEnd(char*);

// line height is determined from [0]th character
struct Font_Struct {
	SDL_Surface *bmps[char_count];
	int space; // Probela lielums pikseïos

	Font_Struct() {
		for(int temp = 0; temp < char_count; temp++) 
			bmps[temp] = NULL;
		space = 1;
	}

	Font_Struct(const char *name, Uint32 color, Uint32 mask) {
		for(int temp = 0; temp < char_count; temp++) 
			bmps[temp] = NULL;
		space = 1;
		initFont(name, color, mask);
	}

	int initFont(const string &name, Uint32 color, Uint32 mask);

	void writeChar(SDL_Surface *screen, unsigned char chn, int x, int y, SDL_Rect *rect);

	void writeStr(SDL_Surface *screen,const char *string, int x, int y, SDL_Rect *rect);
	
	void getCharRect(unsigned char chn, int x, int y, SDL_Rect *rect);

	void strRect(const char *string, SDL_Rect *rect);

	void setSpace(int newSpace) { space = newSpace; }

	void writeStrWrapped(
		SDL_Surface *screen, 
		const char *string,
		int x, 
		int y, 
		SDL_Rect *rect,
		int rightIndent = 0, 
		int leftIndent = 0, 
		int verticalIncrement = 0);

	void strRectWrapped(
	    SDL_Surface *screen, 
		const char *string, 
		int x, 
		int y, 
		SDL_Rect *rect,
		int rightIndent = 0,
		int leftIndent = 0, 
		int verticalIncrement = 0);

	~Font_Struct() {
		for(int temp = 0; temp < char_count; temp++) 
			if (bmps[temp]) freeImage(bmps[temp]);
	}

};

#endif
