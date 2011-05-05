#include "game.h"
#include "structs.h"

int myBlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {

	if(dst != screen && src != screen) {
		return SDL_BlitSurface(src, srcrect, dst, dstrect);
	} else if(src == screen) {
		myFillRect(dst, dstrect, 0);
	}

	int x = 0, y = 0, w = 0, h = 0;

	if(dstrect) {
		x = dstrect->x;
		y = dstrect->y;
		w = dstrect->w;
		h = dstrect->h;
	} else {
		x = 0;
		y = 0;
		w = src->w;
		h = src->h;
	}

	glColor4f(1.0f, 1.0f, 1.0f,1);

	map<SDL_Surface*, ImageStruct*>::iterator i = loadedImages.find(src);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(i == loadedImages.end()) {
		//dout << "Texture not present for: " << src << endl;

		glDisable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glVertex2f(x, y);		
		glVertex2f(x + w, y);		
		glVertex2f(x + w, y + h);		
		glVertex2f(x, y + h);		
		glEnd();

		glEnable(GL_TEXTURE_2D);
	} else {
		ImageStruct *img = i->second;

		//dout << "Must blit: " << img->fileName << ", texture id= " << img->texture << endl;
		//dout << "x: " << x << ", y: " << y << ", w: " << w << ", h: " << h << endl;

		glBindTexture(GL_TEXTURE_2D, img->texture);

		glBegin(GL_QUADS);

		glTexCoord2f(0.0f, 0.0);
		glVertex2f(x, y);		
		glTexCoord2f(img->textureXCoord, 0.0);
		glVertex2f(x + w, y);		
		glTexCoord2f(img->textureXCoord, img->textureYCoord);
		glVertex2f(x + w, y + h);		
		glTexCoord2f(0.0f,  img->textureYCoord);
		glVertex2f(x, y + h);		

		glEnd();
	}


	
	return 0;
}

int myBlitTexture(GLuint tex, float xofs, float yofs, SDL_Rect *dstrect, SDL_Rect *srcrect, Uint16 width, Uint16 height) {
	int x = 0, y = 0, w = 0, h = 0;
	float sx = 0.0;
	float sy = 0.0;
	float dx = xofs;
	float dy = yofs;

	if(dstrect) {
		x = dstrect->x;
		y = dstrect->y;
		w = dstrect->w;
		h = dstrect->h;
	} else {
		x = 0;
		y = 0;
		w = screen->w;
		h = screen->h;
	}

	if(srcrect && width && height) {
		sx = (float)srcrect->x  / width * dx;
		dx = ((float)(srcrect->x + srcrect->w) / width) * dx;

		sy = dy - ((float)(srcrect->y + srcrect->h) / height) * dy;
		dy = dy - (float)srcrect->y  / height * dy;
	}

	glBlendFunc (GL_ONE, GL_ZERO);

	glEnable(GL_TEXTURE_2D);	

	glColor4f(1.0f, 1.0f, 1.0f,1);

	glBindTexture(GL_TEXTURE_2D, tex);

	glBegin(GL_QUADS);

	glTexCoord2f(sx, dy);
	glVertex2f(x, y);		
	glTexCoord2f(dx, dy);
	glVertex2f(x + w, y);		
	glTexCoord2f(dx, sy);
	glVertex2f(x + w, y + h);		
	glTexCoord2f(sx,  sy);
	glVertex2f(x, y + h);		

	glEnd();

	return 0;

}

int myFillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 rcolor, float alpha) {
	if(dst != screen) {
		return SDL_FillRect(dst, dstrect, rcolor);
	}

	int x, y, w, h;

	if(dstrect) {
		x = dstrect->x;
		y = dstrect->y;
		w = dstrect->w;
		h = dstrect->h;
	} else {
		x = 0;
		y = 0;
		w = dst->w;
		h = dst->h;
	}

	float r = ((float)((rcolor >> 16) & 0xFF)) / 256.0;
	float g = ((float)((rcolor >> 8) & 0xFF)) / 256.0;
	float b = ((float)((rcolor) & 0xFF)) / 256.0;

	if(alpha != 1.0) {
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glBlendFunc(GL_ONE, GL_ZERO);
	}

	glColor4f(r,g,b,alpha);

	glDisable(GL_TEXTURE_2D);


	glBegin(GL_QUADS);

	glVertex2f(x, y);		
	glVertex2f(x + w, y);		
	glVertex2f(x + w, y + h);		
	glVertex2f(x, y + h);		

	glEnd();

	glEnable(GL_TEXTURE_2D);

	return 0;
}