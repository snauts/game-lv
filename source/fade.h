#ifndef __FADE_H__
#define __FADE_H__

#include <SDL.h>

extern bool fadeEffectsRandomFactor;

bool doMoveScreenRight(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doMoveScreenLeft(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doMoveScreenUp(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doMoveScreenDown(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doCheckerEffectHorizontal(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doCheckerEffectVertical(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doFadeIntoBox(int method, int speed, int &progress, SDL_Surface *savedScreen);
bool doCrossFade(int method, int speed, int &progress, SDL_Surface *savedScreen);

#endif
