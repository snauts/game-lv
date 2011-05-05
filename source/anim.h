#ifndef __ANIM_H__
#define __ANIM_H__

#include <SDL.h>
#include "structs.h"

Animation* LoadAnimation(const string& file_name, bool useVideoMemory = true, bool loadImageData = true);

unsigned DrawAnimation(SDL_Surface *dst, Animation *anim, int x, int y);
unsigned DrawAnimationClipped(SDL_Surface *dst, int viewx, int viewy, Animation *anim, int x, int y);
unsigned DrawAnimationScaled(SDL_Surface *dst, Animation *anim, int x, int y, double scaleCoef, bool antialiase = false);
unsigned UpdateAnimation(Animation *anim);
void resetAnimation(Animation *anim);
int freeAnimationData(Animation *anim);
int loadAnimationData(Animation *anim);

inline SDL_Surface *getCurrentFrame(Animation *anim) {
	return 	anim->currentBlock->img[anim->currentFrame];
}

#endif
