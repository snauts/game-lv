#include "game.h"
#include "gamestrings.h"

int testMain(int argc, char **argv) {
	FILE *fin = fopen("scripts/startup_script", "r");
	if(!fin) {
		dout << "Bla bla... :)" << strerror(errno) << endl;
		return 1;
	}
	loadSymbolsMap();
	return 0;
}

int initGL(int width, int height);

// Artuurs: Ja gribi testeet whatever ko, tad aizvaac komentu no apaksheejaas #define's
//  un raksti savu 'main' f-jas kodu ieksh 'testMain' f-jas.
// #define __ARCHAZ_TEST__

int gameLV_main(int argc, char** argv) {
#ifdef __ARCHAZ_TEST__
	return testMain(argc, argv);
#endif

	time_t time_val = time(NULL);
	dout << "Started in: " << asctime(localtime(&time_val)) << endl;
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		dout << "Could not initialize SDL" << endl << "Reason : " << SDL_GetError() << endl;
		return -1;
	}
	int screenWidth = 640;
	int screenHeight = 480;
	int bpp = 32;
	int defaultAction = -1;
	//int defaultAction = MENU_NEW_GAME;
	unsigned video_flags =  SDL_OPENGL;
	atexit(SDL_Quit);
	dout << "Processing command line..." << endl;
	char argBuf[256];
	for(int a = 1; a < argc; a++) {
		dout << "Argument [" << a << "], value: '" << argv[a] << "'" << endl;
		strcpy(argBuf, argv[a]);

		if(!strcmp("-f", argBuf)) {

		        video_flags |= SDL_FULLSCREEN;
				dout << "Fullscreen enabled..." << endl;
		} else if(!strcmp("-a", argBuf)) {
			doCharacterAntialiasing = true;
			dout << "Character antialiasing enabled..." << endl;
		} else if(!strcmp("-x", argBuf)) {
			screenWidth = atol(argv[a + 1]);
			a++;
			dout << "Resolution x: "  << screenWidth << endl;
		} else if(!strcmp("-y", argBuf)) {
			screenHeight = atol(argv[a + 1]);
			a++;
			dout << "Resolution y: "  << screenWidth << endl;
		} else if(!strcmp("-new", argBuf)) {
			defaultAction = MENU_NEW_GAME;
			dout << "Will start new game by default" << endl;
		} else if(!strcmp("-dbgout", argBuf)) {
			debugConsole = true;
			dout << "On-screen debug output enabled" << endl;
		} else {
			dout << "Unknown flag..." << endl;
		}
	}

	screen = SDL_SetVideoMode(screenWidth, screenHeight, bpp, video_flags);
	if (screen == NULL) {
		dout << "Could not set video mode: " << SDL_GetError() << endl;
		return 1;
	}

	initGL(screenWidth, screenHeight);

	dout << "screen rmask: " << hex << screen->format->Rmask << dec << endl;
	dout << "screen gmask: " << hex << screen->format->Gmask << dec << endl;
	dout << "screen bmask: " << hex << screen->format->Bmask << dec << endl;
	dout << "screen amask: " << hex << screen->format->Amask << dec << endl;
	dout << "bpp: " << (int)screen->format->BitsPerPixel << endl;
	
	SDL_Surface* icon_bmp = SDL_LoadBMP("icon.bmp");
#ifdef _DEBUG
	SDL_WM_SetCaption("GameLV OpenGL [debug]", NULL);
#else
	SDL_WM_SetCaption("GameLV OpenGL", NULL);
#endif
	if(icon_bmp) {
		SDL_WM_SetIcon(icon_bmp, NULL);
	}


	SDL_ShowCursor(0);
	
	if(Mix_OpenAudio(44100,AUDIO_S16,2,2048) < 0) { //22kHz->44kHz (barvins)
		dout << "Could not initialize mixer: " << SDL_GetError() << endl;
		return -1;
	} 
	else {
		isSoundPossible = true;
		atexit(Mix_CloseAudio);
		int rate, chanels;
		unsigned short format;
		Mix_QuerySpec(&rate, &format, &chanels);
		dout << "Mixer at " << rate << "Hz " << ((chanels > 1) ? "Stereo" : "Mono") << endl;
	}

	// Loading symbols
	loadSymbolsMap();

	srand((unsigned)time(NULL));
	SDL_SetTimer(10, TimerCallback);

	genericInit();

	bool shouldExit = false;
	int ret;
	int resumeRet = 0;
	int param;
	bool canResumeGame = false;
	while(!shouldExit) {
		Mix_Pause(-1);
		Mix_HaltMusic();
		ret = doGameMenu(param, canResumeGame, defaultAction);
		defaultAction = -1;
		dout << "doGameMenu returned: ";
		switch(ret) {
		case MENU_NEW_GAME:
			{
				dout << "Start new game" << endl;
				if(canResumeGame) {
					resetGame();
				}
				newGameInit();
				doStartupScript();
				dout << game_states << endl;
				SDL_WarpMouse(screen->w / 2, screen->h / 2);
				break;
			}
		case MENU_SAVE_GAME:
			{
				dout << "Save game" << endl;

				char filePrefix[256];
				if(saveGameDir.length() > 0) {
					sprintf(filePrefix, "%s" SEPERATOR "%.2i_", saveGameDir.c_str(), param);
				} else {
					sprintf(filePrefix, "%.2i_", param);
				}
				saveGame(filePrefix);
				musicPlayer.resumeMusic();
				break;
			}
		case MENU_LOAD_GAME:
			{
				dout << "Load game" << endl;
				myFillRect(screen, NULL, 0);
				SDL_Rect r;
				normalFont->strRect(STRING_LOADING_GAME, &r);
				normalFont->writeStr(screen, STRING_LOADING_GAME, (screen->w - r.w) / 2, (screen->h - r.h) / 2, NULL);

				SDL_GL_SwapBuffers();

				char filePrefix[256];
				if(saveGameDir.length() > 0) {
					sprintf(filePrefix, "%s" SEPERATOR "%.2i_", saveGameDir.c_str(), param);
				} else {
					sprintf(filePrefix, "%.2i_", param);
				}
				
				if(loadGame(filePrefix)) {
					dout << "ERROR- Could not load game" << endl;
					exit(1);
				}
				
				break;
			}
		case MENU_EXIT:
			{
				dout << "Exit game" << endl;
				shouldExit = true;
				break;
			}
		case MENU_RESUME_GAME:
			{
				musicPlayer.resumeMusic();
				dout << "Resume game" << endl;
				break;
			}
		case MENU_CREDITS:
			{
				break;
			}
		default:
			{
				dout << "ERROR - returned unknown action: " << ret << endl;
				exit(1);
				break;
			}
		}
		if(!shouldExit) {
			Mix_Resume(-1);
			Mix_ResumeMusic();
			resumeRet = ResumeGame();
			if(resumeRet == 0) {
				canResumeGame = true;
			} else {
				canResumeGame = false;
			}
			if(resumeRet == -1) {
				shouldExit = true;
			}
		}
	}
	Mix_HaltChannel(-1);
	Mix_HaltMusic();
	dout << endl << "Finished in: " << asctime(localtime(&time_val)) << endl;

	return 0;
}

int initGL(int width, int height) {

  glViewport(0, 0, width, height);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// This Will Clear The Background Color To Black

  glDisable( GL_LIGHTING );

  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();				// Reset The View

  gluOrtho2D(0, width, height, 0);

  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();				// Reset The View

  glEnable(GL_BLEND);									// Enable Blending
  
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_LINE_SMOOTH);
  //glBlendFunc (GL_ONE, GL_ZERO);

  glEnable(GL_TEXTURE_2D);							// Enable 2D Texture Mapping

  return 0;
}

#ifdef __MINGW32__
#include <windows.h>
BOOL WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInst,
		    LPSTR cmdLine, int cmdShow) {
    gameLV_main(_argc, _argv);
    return true;
}
#else
int main(int argc, char** argv) {
    return gameLV_main(argc, argv);
}
#endif
