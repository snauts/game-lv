#include "game.h"

map<SDL_Surface*, ImageStruct*> loadedImages;

ImageStruct *isImageLoaded(const string& img, bool has_color_key, int color_key, 
						   bool has_alpha, int opacity, bool shouldConvert, bool useVideoMemory) {

	map<SDL_Surface*, ImageStruct*>::iterator i = loadedImages.begin();
	ImageStruct *imageStruct;
	bool isFile;
	bool isAlpha;
	bool isColorKey;
	bool isVideoMem;
	bool isConvert;

	while(i != loadedImages.end()) {
		imageStruct = i->second;

		isFile = false;
		isAlpha = false;
		isColorKey = false;
		isVideoMem = false;
		isConvert = false;

		if(imageStruct->fileName == img) {
			isFile = true;
		}
		if(imageStruct->hasColorKey == has_color_key) {
			if(has_color_key) {
				if(imageStruct->colorKey == color_key) {
					isColorKey = true;
				}
			} else {
				isColorKey = true;
			}
		}
		if(imageStruct->hasAlpha == has_alpha) {
			if(has_alpha) {
				if(imageStruct->alpha == opacity) {
					isAlpha = true;
				}
			} else {
				isAlpha = true;
			}
		}
		if(imageStruct->useVideoMemory == useVideoMemory) {
			isVideoMem = true;
		}
		if(imageStruct->shouldConvert == shouldConvert) {
			isConvert = true;
		}

		if(isFile && isColorKey && isAlpha && isVideoMem && isConvert) {
			return imageStruct;
		}
		i++;
	}
	return NULL;
}

void TrimWhitespaces(char* str) {
	unsigned a = 0;
	do {
		if(str[a] <= ' ') {
			str[a] = 0;
			break;
		}
	} while(str[++a]);
}

int createTextureFromSurface(ImageStruct *img) {

	int i, j;

	static bool init = false;
	static SDL_Surface *refSurface = NULL;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int rmask = 0xff000000;
	int gmask = 0x00ff0000;
	int bmask = 0x0000ff00;
	int amask = 0x000000ff;
#else
	int rmask = 0x000000ff;
	int gmask = 0x0000ff00;
	int bmask = 0x00ff0000;
	int amask = 0xff000000;
#endif
/*
	int rmask = screen->format->Rmask;
	int gmask = screen->format->Gmask;
	int bmask = screen->format->Bmask;
	int amask = screen->format->Amask;
*/
	int w = img->img->w;
	int h = img->img->h;

	i = 1;
	while(i < w) { i*=2; }
	w = i;

	i = 1;
	while(i < h) { i*=2; }
	h = i;

	//dout << "Name: " << img->fileName << ", Texture size: " << w << "," << h << endl;

	//SDL_Surface *tmp2 = SDL_ConvertSurface(img->img, screen->format, SDL_SWSURFACE);
	SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, w, h, 32, rmask, gmask, bmask, amask);

	if(!tmp /*|| !tmp2*/) {
        fprintf(stderr, "CreateRGBSurface failed for ref_surface: %s\n", SDL_GetError());
		exit(1);
	}

	bool hasAlpha = img->hasAlpha;
	float imgAlpha = (float)img->alpha / 100.0;
	bool hasColorKey = img->hasColorKey;
	Uint32 colorKey = img->colorKey;

	// If per-pixel alpha is used we must manually copy data, as SDL does not supports it and looses alpha info
	// Per pixel alpha is only supported in 32bpp colors
	if(hasAlpha && img->alpha < 0) {
		if(img->img->format->BitsPerPixel == 32) {
			unsigned char *srcpix = (unsigned char*) img->img->pixels;
			unsigned char *dstpix = (unsigned char*) tmp->pixels;

			for(int a = 0; a < img->img->w; a++) {
				memcpy(dstpix, srcpix, img->img->pitch);
				srcpix += img->img->pitch;
				dstpix += tmp->pitch;
			}
		} else {
			SDL_BlitSurface(img->img, NULL, tmp, NULL);
		}
	} else {
		SDL_BlitSurface(img->img, NULL, tmp, NULL);
	}

	if(img->hasColorKey && colorKey == -1) {
		colorKey = (*((Uint32*)tmp->pixels)) & 0xffffff;
	}

	unsigned char *row = (unsigned char*)tmp->pixels;

	for(i = 0; i < tmp->h; i++) {
		Uint32 *cur = (Uint32*)row;

		for(j = 0; j < tmp->w; j++) {

			Uint32 v = *cur;
			Uint32 col = v & 0xffffff;
			unsigned char alpha = 255;

			if(hasColorKey && (col == colorKey)) {
				alpha = 0;
				col = 0;
			}

			if(hasAlpha) {
				if(img->alpha >= 0) {
					alpha = 255 * imgAlpha;
				} else {
					alpha = (v & 0xff000000) >> 24;
				}
			}
			
			*cur = col | (alpha << 24);

			cur++;
		}
		row += tmp->pitch;
	}

	glGenTextures(1, &img->texture);

	img->textureXCoord = (float)img->img->w / (float)w;
	img->textureYCoord = (float)img->img->h / (float)h;

	glBindTexture(GL_TEXTURE_2D, img->texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	SDL_FreeSurface(tmp);
	//SDL_FreeSurface(tmp2);

	return 0;
}

SDL_Surface* loadImage(const string& img, bool has_color_key, int color_key, 
					   bool has_alpha, int opacity, bool shouldConvert, bool useVideoMemory, bool shouldCache, bool requiresTexture) {


	useVideoMemory = false;

	ImageStruct *imageStruct = isImageLoaded(img, has_color_key, color_key,
		has_alpha, opacity, shouldConvert, useVideoMemory);

	if(imageStruct && shouldCache) {
	    // dout << "OPTIMIZE: image already in memory: " << img << endl;
		imageStruct->refCount++;
		return imageStruct->img;
	}

	SDL_Surface *tmp = IMG_Load(convertPath(img).c_str());

	//dout << "Image: " << img << ", bpp: " << tmp->format->BitsPerPixel << endl;

	if(!tmp) return NULL;

	if(has_color_key) {

		int src_color_key = color_key;
		int dst_color_key;
		Uint8 key_r, key_g, key_b;

		if(src_color_key == -1) {
			src_color_key = 0;

			memcpy(&src_color_key, tmp->pixels, tmp->format->BytesPerPixel);

			SDL_GetRGB(src_color_key, tmp->format, &key_r, &key_g, &key_b);

			dst_color_key = SDL_MapRGB(screen->format, key_r, key_g, key_b);
					
			char maskBuf[256];
			sprintf(maskBuf, "%s : %i bits -> %i bits, %X -> %X, r : %X, g : %X, b : %X", img.c_str(), tmp->format->BitsPerPixel,
				screen->format->BitsPerPixel, src_color_key, dst_color_key, key_r, key_g, key_b);
			dout << maskBuf << endl;
			dout << "  Src color key: " << hex << src_color_key << dec << endl;
			dout << "  Dst color key: " << hex << dst_color_key << dec << endl;

			color_key = dst_color_key;
		}
	}


	if(has_alpha) {
		if(opacity > 100) opacity = 100;
		//if(opacity < 0)	opacity = 0;
	}


	if(shouldCache) {
		imageStruct = new ImageStruct;
		imageStruct->img = tmp;
		imageStruct->fileName = img;
		imageStruct->hasColorKey = has_color_key;
		imageStruct->colorKey = color_key;
		imageStruct->hasAlpha = has_alpha;
		imageStruct->alpha = opacity;
		imageStruct->shouldConvert = shouldConvert;
		imageStruct->useVideoMemory = useVideoMemory;
		imageStruct->refCount = 1;

		if(requiresTexture) {
			createTextureFromSurface(imageStruct);
		} else {
			dout << "Texture not required for: " << img << endl;
			imageStruct->texture = 0;
		}

		loadedImages.insert(make_pair<SDL_Surface*, ImageStruct*>(tmp, imageStruct));
	}

	return tmp;
}

void freeImage(SDL_Surface *surf) {
	if(!surf) {
		dout << "ERROR- freeImage received NULL surface" << endl;
		exit(1);
	}
	map<SDL_Surface*, ImageStruct*>::iterator i = loadedImages.find(surf);
	if(i == loadedImages.end())	{
		dout << "ERROR- freeImage called for unknown surface!!!" << endl;
		exit(1);
	}
	ImageStruct *imageStruct = i->second;

	imageStruct->refCount--;

//	dout << "freeImage: refrence count for: " << imageStruct->fileName << " is " << imageStruct->refCount << endl;

	if(imageStruct->refCount <= 0) {
		if(imageStruct->texture) glDeleteTextures(1, &imageStruct->texture);
		SDL_FreeSurface(surf);
		loadedImages.erase(i);
	}
}

unsigned char* loadMask(const string& mask_file, int& w, int& h) {
	SDL_Surface* s = loadImage(mask_file, false, 0, false, 0, false, false, true, false);
	unsigned char* buf;
	if(!s) {
		dout << "Could not load mask image: " << mask_file << endl;
		return NULL;
	}
	
	if((s->format->BitsPerPixel % 8) != 0) {
		dout << "Mask: " << mask_file << " has invalid bpp size: " << (int)s->format->BitsPerPixel << endl;
		freeImage(s);
		return NULL;
	}
	buf = new unsigned char[s->w * s->h];
	int lineInBytes = s->pitch;
	w = s->w;
	h = s->h;
	memset(buf, 0, sizeof(unsigned char) * w * h);
	if((s->flags & SDL_HWSURFACE) == SDL_HWSURFACE) {
		SDL_LockSurface(s);
	}
	unsigned mult = s->format->BytesPerPixel;
	unsigned val;
	unsigned a, b;
	for(a = 0; a < h; a++) {
		for(b = 0; b < w; b++) {
			val = 0;
			memcpy(&val,(void*)((unsigned long)s->pixels + (a * lineInBytes + b * mult)), mult);
			buf[a * w + b] = val & 0xff;
		}
	}
	if((s->flags & SDL_HWSURFACE) == SDL_HWSURFACE) {
		SDL_UnlockSurface(s);
	}
	freeImage(s);
	return buf;
}



