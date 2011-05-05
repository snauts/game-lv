#include "game.h"

//-----------------------------------------------------------------
// bool isStringEnd(char *string)
// bool isSpace(char *string)
// bool isEnter(char *string)
//
// Functions for character checking
//-----------------------------------------------------------------

bool isStringEnd(const char *string) {
	if(string[0] == endSymbol)
		return true;
	else
		return false;
}

bool isSpace(const char *string) {
	if(string[0] == spaceSymbol)
		return true;
	else
		return false;
}

/*
bool isLineEnd(const char *string) {
	if(string[0] == backSlash && string[1] == letter)
		return true;
	else
		return false;
}
*/

//-----------------------------------------------------------------
// void composeRects(SDL_Rect *rect1, SDL_Rect *rect2)
//
// This function composes rectangle from rect1 & rect2 
// and leaves result in rect1.
//-----------------------------------------------------------------

void composeRects(SDL_Rect *rect1, SDL_Rect *rect2) {
	SDL_Rect temp;
	
	if (rect1->x > rect2->x) temp.x = rect2->x; else temp.x = rect1->x;
	
	if (rect1->y > rect2->y) temp.y = rect2->y; else temp.y = rect1->y;

	if (rect1->x + rect1->w > rect2->x + rect2->w) 
		temp.w = rect1->x + rect1->w; 
	else 
		temp.w = rect2->x + rect2->w;

	if (rect1->y + rect1->h > rect2->y + rect2->h) 
		temp.h = rect1->y + rect1->h; 
	else 
		temp.h = rect2->y + rect2->h;

	temp.w -= temp.x;
	temp.h -= temp.y;

	// copy temp on rect1
	rect1->x = temp.x;
	rect1->y = temp.y;
	rect1->w = temp.w;
	rect1->h = temp.h;
}

/******************************************************************************************/

// Ja color == mask tiek veidoc normâls fonc.
// Ja color != mask, fonts tiek veidots color krâsâ,
// BET ðajâ gadîjumâ (c != m) izðíirðanas dziïumam (bpp) ir 
// jâbût == ar fontu bildîðu (bitmapu) dziïumiem.

static SDL_Surface *loadFontFile(const char *formatStr,
				 const string &name,
				 Uint32 mask,
				 int npk) {
    char filename[256];
    sprintf(filename, formatStr, name.c_str(), npk);
    return loadImage(filename, true, mask, false, 100, true, true, true);
}

int Font_Struct::initFont(const string &name, Uint32 color, Uint32 mask) {
	unsigned tmpMask;
	unsigned tmpColor;

	for(int temp = 0; temp < char_count; temp++) {
		if (bmps[temp]) freeImage(bmps[temp]);

		bmps[temp] = loadFontFile("%s%5.5u.bmp", name, mask, temp);
		if (!bmps[temp]) {
		    bmps[temp] = loadFontFile("%s%5.5u.png", name, mask, temp);
		}

		if ((color != mask) && (bmps[temp] != NULL)) {

			tmpMask = bmps[temp]->format->colorkey;
			tmpColor = SDL_MapRGB(bmps[temp]->format, (color & 0xff0000) >> 16,
				(color & 0xff00) >> 8, color & 0xff);
			
			Uint8 bpp = bmps[temp]->format->BytesPerPixel;

			Uint8  *ptr8  = (Uint8*)  bmps[temp]->pixels;
			Uint16 *ptr16 = (Uint16*) bmps[temp]->pixels;
			Uint32 *ptr32 = (Uint32*) bmps[temp]->pixels;

			for(int offset = 0; offset < (bmps[temp]->h * (bmps[temp]->pitch / bpp)); offset++) {
				switch(bpp) {
				case 1: 
					{ 
						if (ptr8[offset] != (Uint8) mask) ptr8[offset] = (Uint8) color;
						break; 
					}
					
				case 2: 
					{
						if (ptr16[offset] != (Uint16) tmpMask) 
							ptr16[offset] = (Uint16) tmpColor;
						break; 
					}
					
				case 3: 
					{ 
						// 24 bits are for losers
						break; 
					}
					
				case 4: 
					{ 
						if (ptr32[offset] != (Uint32) mask) ptr32[offset] = (Uint32) color;
						break; 
					}
				}
			}
		}

	}
	return 0;	
}

/******************************************************************************************/

void Font_Struct::writeChar(SDL_Surface *screen, unsigned char chn, int x, int y, SDL_Rect *rect) {
	bool newRect = false;
	if(!rect) {
		rect = new SDL_Rect;
		newRect = true;
	}
	
	rect->x = x; 
	rect->y = y; 
	rect->w = bmps[chn]->w; 
	rect->h = bmps[chn]->h;
	
	SDL_Rect srcRect = {0, 0, rect->w, rect->h};
	
	if(rect->x < 0) {
		srcRect.w += rect->x;
		srcRect.x = abs(rect->x);
		rect->w += rect->x;
		rect->x = 0;
	}

	if(rect->y < 0) {
		srcRect.h += rect->y;
		srcRect.y = abs(rect->y);
		rect->h += rect->y;
		rect->y = 0;
	}

	myBlitSurface(bmps[chn], &srcRect, screen, rect);

	if (newRect) delete rect;
}

/******************************************************************************************/

void Font_Struct::writeStr(SDL_Surface *screen, const char *string, int x, int y, SDL_Rect *rect) {
	bool newRect = false;
	if(!rect) {
		rect = new SDL_Rect;
		newRect = true;
	}

	SDL_Rect *charRect = new SDL_Rect; 
	unsigned char tc;
	int cursor_x = x, cursor_y = y;
	
	rect->x = x; 
	rect->y = y; 
	rect->w = 0; 
	rect->h = 0; 

	for (unsigned temp = 0; temp < strlen(string); temp++) {
		tc = (unsigned char) string[temp]; 

		if(string[temp] == lineFeed) {
			cursor_y += bmps[0]->h;
			continue;
		}
		

		if(string[temp] == carriage) {
			cursor_x = x;
			continue;
		}

		if (bmps[tc] != NULL) {
			writeChar(screen, tc, cursor_x, cursor_y, charRect);
			composeRects(rect, charRect);
			cursor_x = cursor_x + space + bmps[tc]->w;	
		}
	}

	delete charRect;
	if (newRect) delete rect;
}

/******************************************************************************************/

void Font_Struct::getCharRect(unsigned char chn, int x, int y, SDL_Rect *rect) {
	bool newRect = false;
	if(!rect) {
		rect = new SDL_Rect;
		newRect = true;
	}

	rect->x = x; 
	rect->y = y; 
	rect->w = bmps[chn]->w; 
	rect->h = bmps[chn]->h;
	
	if(rect->x < 0) {
		rect->w += rect->x;
		rect->x = 0;
	}

	if(rect->y < 0) {
		rect->h += rect->y;
		rect->y = 0;
	}

	if (newRect) delete rect;
}

/******************************************************************************************/

void Font_Struct::strRect(const char *string, SDL_Rect *rect) {
	bool newRect = false;
	if(!rect) {
		rect = new SDL_Rect;
		newRect = true;
	}

	SDL_Rect *charRect = new SDL_Rect;
	int cursor_x = 0, cursor_y = 0;
	
	rect->x = 0; 
	rect->y = 0; 
	rect->w = 0; 
	rect->h = 0; 

	unsigned char tc;

	for (unsigned temp = 0; temp < strlen(string); temp++) {
		tc = (unsigned char) string[temp]; 
		
		if(string[temp] == lineFeed) {
			cursor_y += bmps[0]->h;
			continue;
		}

		if(string[temp] == carriage) {
			cursor_x = 0;
			continue;
		}

		if (bmps[tc] != NULL) {
			getCharRect(tc, cursor_x, cursor_y, charRect);
			composeRects(rect, charRect);
			cursor_x = cursor_x + space + bmps[tc]->w;	
		}
	}

	delete charRect;
	if (newRect) delete rect;
}

/******************************************************************************************/

void Font_Struct::writeStrWrapped(
	SDL_Surface *screen, 
	const char *string,
	int x, 
	int y, 
	SDL_Rect *rect,
	int rightIndent, 
	int leftIndent, 
	int verticalIncrement) {

	bool newRect = false;
	if(!rect) {
		rect = new SDL_Rect;
		newRect = true;
	}

	rect->x = x; 
	rect->y = y; 
	rect->w = 0; 
	rect->h = 0; 

	SDL_Rect *wordRect = new SDL_Rect;
	char *word = new char[strlen(string)];
	int cursor_x = x, cursor_y = y;
	bool firstWord = true, stringEnd = false; 
	bool isLineFeed, isCarriage; 
	int position = 0, wordPosition = 0; 
	int spaceLeft = screen->w - rightIndent - x;

	while(!stringEnd) {
		isLineFeed = false;
		isCarriage = false;
		while(!isSpace(string + position)) {		
			if(string[position] == lineFeed) {
				isLineFeed = true;
				break;
			}

			if(string[position] == carriage) {
				isCarriage = true;
				break;
			}

			if(isStringEnd(string + position)) {
				stringEnd = true;
				break;
			}

			word[wordPosition] = string[position];
			position++;
			wordPosition++;

			if(isSpace(string + position)) {
				word[wordPosition] = spaceSymbol;
				wordPosition++;
			}
		}

		position++;
		word[wordPosition] = endSymbol;

		strRect(word, wordRect);

		if(!firstWord && wordRect->w > spaceLeft) {
			cursor_x = x - leftIndent;
			cursor_y += bmps[0]->h + verticalIncrement;
			spaceLeft = screen->w - rightIndent - x + leftIndent;
		} 

		writeStr(screen, word, cursor_x, cursor_y, wordRect);
		composeRects(rect, wordRect);

		cursor_x += wordRect->w;
		spaceLeft -= wordRect->w;
		
		firstWord = false;

		if (isLineFeed) {
			cursor_y += bmps[0]->h + verticalIncrement;
			spaceLeft = screen->w - rightIndent - x + leftIndent;
			firstWord = true;
		}

		if (isCarriage) {
			cursor_x = x - leftIndent;
			spaceLeft = screen->w - rightIndent - x + leftIndent;
			firstWord = true;
		}

		wordPosition = 0;
	}

	delete [] word;
	delete wordRect;
	if (newRect) delete rect;
	}

/******************************************************************************************/

void Font_Struct::strRectWrapped(
    SDL_Surface *screen, 
	const char *string, 
	int x, 
	int y, 
	SDL_Rect *rect,
	int rightIndent,
	int leftIndent, 
	int verticalIncrement) {

	bool newRect = false;
	if(!rect) {
		rect = new SDL_Rect;
		newRect = true;
	}

	rect->x = x; 
	rect->y = y; 
	rect->w = 0; 
	rect->h = 0; 

	SDL_Rect *wordRect = new SDL_Rect;
	char *word = new char[strlen(string)];
	int cursor_x = x, cursor_y = y;
	bool firstWord = true, stringEnd = false;
	bool isLineFeed, isCarriage; 
	int position = 0, wordPosition = 0; 
	int spaceLeft = screen->w - rightIndent - x;

	while(!stringEnd) {
		isLineFeed = false;
		isCarriage = false;
		while(!isSpace(string + position)) {		
			if(string[position] == lineFeed) {
				isLineFeed = true;
				break;
			}

			if(string[position] == carriage) {
				isCarriage = true;
				break;
			}

			if(isStringEnd(string + position)) {
				stringEnd = true;
				break;
			}

			word[wordPosition] = string[position];
			position++;
			wordPosition++;

			if(isSpace(string + position)) {
				word[wordPosition] = spaceSymbol;
				wordPosition++;
			}
		}

		position++;
		word[wordPosition] = endSymbol;

		strRect(word, wordRect);

		if(!firstWord && wordRect->w > spaceLeft) {
			cursor_x = x - leftIndent;
			cursor_y += bmps[0]->h + verticalIncrement;
			spaceLeft = screen->w - rightIndent - x + leftIndent;
		} 

		wordRect->x = cursor_x; 
		wordRect->y = cursor_y;

		composeRects(rect, wordRect);

		cursor_x += wordRect->w;
		spaceLeft -= wordRect->w;
		
		firstWord = false;

		if (isLineFeed) {
			cursor_y += bmps[0]->h + verticalIncrement;
			spaceLeft = screen->w - rightIndent - x + leftIndent;
			firstWord = true;
		}

		if (isCarriage) {
			cursor_x = x - leftIndent;
			spaceLeft = screen->w - rightIndent - x + leftIndent;
			firstWord = true;
		}

		wordPosition = 0;
	}

	rect->x = 0;
	rect->y = 0;
	
	delete [] word;
	delete wordRect;
	if (newRect) delete rect;

	}



