#include "game.h"

bool fadeEffectsRandomFactor = false;

int NORMAL_LINE_COUNT = 10;
int LINE_COUNT = 10;

bool doMoveScreenRight(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;
	if(dif > speed) {
		return false;
	}

	double currProgr = (dif / speed) * screen->w;
	progress = (int) currProgr;
	
	SDL_Rect src;
	SDL_Rect dst;

	src.x = 0;
	src.w = screen->w - progress;
	src.y = 0;
	src.h = screen->h;
	
	dst.x = progress;
	dst.w = src.w;
	dst.y = 0;
	dst.h = screen->h;
	//myBlitSurface(savedScreen, &src, screen, &dst);
	float dx = (float)screen->w / savedScreenTextureW;
	float dy = (float)screen->h / savedScreenTextureH;	
	
	myBlitTexture(savedScreenTexture, dx, dy, &dst, &src, screen->w, screen->h);
	
	return true;
}

bool doCrossFade(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}

	progress = 0;

	float dif = (float)(current_time - startTime);
	if(dif >= speed) {
		return false;
	}


	float currProgr = dif / (float)speed;

	dout << "Crossfade: " << currProgr << "dif: " << dif << ", speed: " << speed << endl;

	if(currProgr < 0.5) {
		float dx = (float)screen->w / savedScreenTextureW;
		float dy = (float)screen->h / savedScreenTextureH;	
	
		myBlitTexture(savedScreenTexture, dx, dy, NULL);
	} else {
		currProgr = 1.0f - currProgr;
	}

	currProgr *= 2.0f;

	myFillRect(screen, NULL, 0, currProgr);


	return true;
}


bool doMoveScreenLeft(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;
	if(dif > speed) {
		return false;
	}

	double currProgr = (dif / speed) * screen->w;
	progress = (int) currProgr;
	
	SDL_Rect src;
	SDL_Rect dst;

	src.x = progress;
	src.w = screen->w - progress;
	src.y = 0;
	src.h = screen->h;

	dst.x = 0;
	dst.w = src.w;
	dst.y = 0;
	dst.h = screen->h;
	//myBlitSurface(savedScreen, &src, screen, &dst);
	float dx = (float)screen->w / savedScreenTextureW;
	float dy = (float)screen->h / savedScreenTextureH;	
	
	myBlitTexture(savedScreenTexture, dx, dy, &dst, &src, screen->w, screen->h);

	return true;
}


bool doCheckerEffectHorizontal(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;

	if(dif > speed) {
		return false;
	}

	double currProgr = (dif / speed) * screen->w;
	progress = (int) currProgr;
	
	int lineHeight = screen->h / (LINE_COUNT * 2);
	SDL_Rect src;
	SDL_Rect dst;
	int a;
	for(a = 0; a < LINE_COUNT; a++) {
		src.x = progress;
		src.w = screen->w - progress;
		src.y = a * lineHeight * 2;
		src.h = lineHeight;
		
		dst.x = 0;
		dst.w = src.w;
		dst.y = a * lineHeight * 2;
		dst.h = lineHeight;
		//myBlitSurface(savedScreen, &src, screen, &dst);
		float dx = (float)screen->w / savedScreenTextureW;
		float dy = (float)screen->h / savedScreenTextureH;
		myBlitTexture(savedScreenTexture, dx, dy, &dst, &dst, screen->w, screen->h);
	}

	for(a = 0; a < LINE_COUNT; a++) {
		src.x = 0;
		src.w = screen->w - progress;
		src.y = a * lineHeight * 2 + lineHeight;
		src.h = lineHeight;
		
		dst.x = progress;
		dst.w = src.w;
		dst.y = a * lineHeight * 2 + lineHeight;
		dst.h = lineHeight;
		//myBlitSurface(savedScreen, &src, screen, &dst);
		float dx = (float)screen->w / savedScreenTextureW;
		float dy = (float)screen->h / savedScreenTextureH;

		myBlitTexture(savedScreenTexture, dx, dy, &dst, &dst, screen->w, screen->h);
	}
	//stopTime();
	//SDL_Delay(100);
	//resumeTime();
	return true;
}

bool doFadeIntoBox(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
		progress = 0;
	}
	double dif = current_time - startTime;
	double currProgr = (dif / speed);
	if(dif > speed) {
		return false;
	}

	int xw = (int) (currProgr * (screen->w / 2));
	int yw = (int) (currProgr * (screen->h / 2));
	SDL_Rect r;
	r.x = xw;
	r.w = screen->w - xw * 2;
	r.y = yw;
	r.h = screen->h - yw * 2;

	//myBlitSurface(savedScreen, &r, screen, &r);
	float dx = (float)screen->w / savedScreenTextureW;
	float dy = (float)screen->h / savedScreenTextureH;	
	
	myBlitTexture(savedScreenTexture, dx, dy, &r);
	return true;
}

bool doMoveScreenDown(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;
	if(dif > speed) {
		return false;
	}
	
	double currProgr = (dif / speed) * screen->h;
	progress = (int) currProgr;
	
	SDL_Rect src;
	SDL_Rect dst;
	
	src.x = 0;
	src.w = screen->w;
	src.y = 0;
	src.h = screen->h - progress;
	
	dst.x = 0;
	dst.w = screen->w;
	dst.y = progress;
	dst.h = src.h;
	//myBlitSurface(savedScreen, &src, screen, &dst);
	float dx = (float)screen->w / savedScreenTextureW;
	float dy = (float)screen->h / savedScreenTextureH;	
	
	myBlitTexture(savedScreenTexture, dx, dy, &dst, &src, screen->w, screen->h);
	return true;	
}

bool doMoveScreenUp(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;
	if(dif > speed) {
		return false;
	}
	
	double currProgr = (dif / speed) * screen->h;
	progress = (int) currProgr;
	
	SDL_Rect src;
	SDL_Rect dst;
	
	src.x = 0;
	src.w = screen->w;
	src.y = progress;
	src.h = screen->h - progress;
	
	dst.x = 0;
	dst.w = screen->w;
	dst.y = 0;
	dst.h = src.h;
	//myBlitSurface(savedScreen, &src, screen, &dst);
	float dx = (float)screen->w / savedScreenTextureW;
	float dy = (float)screen->h / savedScreenTextureH;	
	
	myBlitTexture(savedScreenTexture, dx, dy, &dst, &src, screen->w, screen->h);
	return true;
}

bool doCheckerEffectVertical(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;
	
	if(dif > speed) {
		return false;
	}
	
	double currProgr = (dif / speed) * screen->h;
	progress = (int) currProgr;
	
	int lineWidth = screen->w / (LINE_COUNT * 2);
	SDL_Rect src;
	SDL_Rect dst;
	int a;
	for(a = 0; a < LINE_COUNT; a++) {
		src.x = a * lineWidth * 2;
		src.w = lineWidth;
		src.y = progress;
		src.h = screen->h - progress;
		
		dst.x = a * lineWidth * 2;
		dst.w = lineWidth;
		dst.y = 0;
		dst.h = src.h;
		//myBlitSurface(savedScreen, &src, screen, &dst);
		float dx = (float)screen->w / savedScreenTextureW;
		float dy = (float)screen->h / savedScreenTextureH;
		myBlitTexture(savedScreenTexture, dx, dy, &dst, &dst, screen->w, screen->h);
	}

	for(a = 0; a < LINE_COUNT; a++) {
		src.x = a * lineWidth * 2 + lineWidth;
		src.w = lineWidth;
		src.y = 0;
		src.h = screen->h - progress;
		
		dst.x = a * lineWidth * 2 + lineWidth;
		dst.w = lineWidth;
		dst.y = progress;
		dst.h = src.h;
		//myBlitSurface(savedScreen, &src, screen, &dst);
		float dx = (float)screen->w / savedScreenTextureW;
		float dy = (float)screen->h / savedScreenTextureH;
		myBlitTexture(savedScreenTexture, dx, dy, &dst, &dst, screen->w, screen->h);
	}
	return true;
}


/*
bool doSimpleFade(int method, int speed, int &progress, SDL_Surface *savedScreen) {
	static int startTime;
	if(progress < 0) {
		if(speed <= 0) {
			return false;
		}
		startTime = current_time;
	}
	double dif = current_time - startTime;
	double currProgr = (dif / speed) * SDL_ALPHA_OPAQUE;
	progress = currProgr;
	if(progress > SDL_ALPHA_OPAQUE) {
		return false;
	}
	SDL_SetAlpha(savedScreen, SDL_SRCALPHA, SDL_ALPHA_OPAQUE - progress);
	myBlitSurface(savedScreen, NULL, screen, NULL);
	return true;
}
*/
