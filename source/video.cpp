#include "game.h"


static bool drawVideoFrame = false;

void drawVideo(SDL_Surface *surf, int width, int height);

void videoUpdate(SDL_Surface *screenSurf, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
	//printf("update in %i\n", time(NULL));
	drawVideoFrame = true;	 
}

void playVideo(const string &file) {

	//myFillRect(screen, NULL, 0);
	//SDL_GL_SwapBuffers();
	
    SMPEG *mpeg;
	SMPEG_Info info;
	mpeg = SMPEG_new(file.c_str(), &info, false);
	bool hasAudio = (info.has_audio > 0);
	
	if ( SMPEG_error(mpeg) ) {
		dout << "MPEG error: "  << SMPEG_error(mpeg) << endl;
		exit(1);
	}
	
	int done = 0;

    SDL_Surface *videoSurface = SDL_AllocSurface( SDL_SWSURFACE,
				nearestPow2(info.width),
				nearestPow2(info.height),
				32,
				0x000000FF,
				0x0000FF00,
				0x00FF0000,
				0xFF000000 );

	if ( !videoSurface ) {
		dout << "Failed to allocate memory for video playback" << endl;
		exit(1);
	}

	
	SMPEG_enablevideo(mpeg, 1);

	SDL_mutex *mutex = SDL_CreateMutex();

	SMPEG_setdisplay(mpeg, videoSurface, mutex, videoUpdate );

	SMPEG_scaleXY(mpeg, info.width, info.height);

	//SMPEG_setdisplayregion(mpeg, 0, 0, info.width, info.height);

	//SMPEG_setdisplay(mpeg, screen, NULL, update);

	
	if(hasAudio) {
		SDL_AudioSpec audiofmt;
		Uint16 format;
		int freq, channels;
		
		/* Tell SMPEG what the audio format is */
		Mix_QuerySpec(&freq, &format, &channels);
		audiofmt.format = format;
		audiofmt.freq = freq;
		audiofmt.channels = channels;
		SMPEG_actualSpec(mpeg, &audiofmt);
		
		/* Hook in the MPEG music mixer */
		Mix_HookMusic(SMPEG_playAudioSDL, mpeg);
		SMPEG_enableaudio(mpeg, 1);
		SMPEG_setvolume(mpeg, 100);
	} else {
		Mix_PauseMusic();
		SMPEG_enableaudio(mpeg, 0);
	}
	
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_TEXTURE_2D);
	
	SMPEG_play(mpeg);

    while( !done && SMPEG_status( mpeg ) == SMPEG_PLAYING ) {
        SDL_Event event;

        while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
				case SDL_KEYDOWN:
				{
					if ( event.key.keysym.sym == SDLK_SPACE ) {
						done = 1;
					}
	                break;
				}

				case SDL_QUIT:
				{
					exit(1);
				}

				default:
					break;
            }
        }

		if(drawVideoFrame) {
			SDL_mutexP(mutex);
			drawVideoFrame = false;
			drawVideo(videoSurface, info.width, info.height);
			//printf("draw in %i\n", time(NULL));
			SDL_mutexV(mutex);
		    SDL_GL_SwapBuffers();
		}

    }

	SMPEG_stop(mpeg);

	if(hasAudio) {
		Mix_HookMusic(NULL, NULL);
	} else {
		Mix_ResumeMusic();
	}

	SDL_FreeSurface(videoSurface);

	myFillRect(screen, NULL, 0);
    SDL_GL_SwapBuffers();
	
	SMPEG_delete(mpeg);
}

void drawVideo(SDL_Surface *surf, int width, int height) {

	unsigned char *p = (unsigned char*) surf->pixels;

	static bool initialised = false;

	static GLuint myTexture = 0;

	if(!initialised) {
		glGenTextures(1, &myTexture);

		glBindTexture(GL_TEXTURE_2D, myTexture);

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		initialised = true;
	}

	glBindTexture( GL_TEXTURE_2D, myTexture );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);

	dout << "width: " << surf->w << ", height: " << surf->h << endl;

	static int w = screen->w;
	static int h = screen->h;

	dout << "s width: " << w << ", height: " << h << endl;

	glColor4f(1.0, 1.0, 1.0, 1.0);

	float dx = (float)width / (float)surf->w;
	float dy = (float)height / (float)surf->h;


	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f(0,0);
		glTexCoord2f(dx, 0.0);
		glVertex2f(w,0);
		glTexCoord2f(dx, dy);
		glVertex2f(w,h);
		glTexCoord2f(0.0, dy);
		glVertex2f(0,h);
	glEnd();

	/*
	glPixelStorei( GL_UNPACK_ROW_LENGTH, surf->w );
	glPixelStorei( GL_UNPACK_SKIP_ROWS, textures[i].skip_rows );
	glPixelStorei( GL_UNPACK_SKIP_PIXELS, textures[i].skip_pixels );
	*/
	

}
