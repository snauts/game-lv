
#ifndef __MUSIC_H__
#define __MUSIC_H__

#include "game.h"

struct SongCollection {
	int id;
	vector<string> songFiles;
	int nextSongMode;
	int volume;
	SongCollection() {
		id = -1;
		nextSongMode = -1;
		volume = 64;
	}
};

class MusicPlayer {
public:
	MusicPlayer();
	~MusicPlayer();

	bool updateSpecialEffect();
	int doSpecialEffect(int effect, int time, int param);

	int loadSongs(const string &songFile);
	int playMusic(int id, int resumeMode, bool shouldSetVolume = true);
	int pauseMusic();
	int resumeMusic();

	int stopMusic();
	bool isPlaying();
	int setVolume(int volume);
	int playNext(bool shouldSetVolume = true);

	int saveMusic(FILE *fout);
	int loadMusic(FILE *fin);

	bool isMusicEffectsOn;
	
private:
	map<int, SongCollection*> allSongs;
	SongCollection *currentCollection;
	vector<string>::iterator currentSongFile;
	Mix_Music *currentMusic;
	int volume;
	
	static void musicFinishedHandler(void);
	static MusicPlayer *currentMusicPlayer;

	int musicEffect;
	int endTime;
	int startTime;
	int initialParam;
	int initialTime;
	int prevVolume;
	int startVolume;
	int effectState;
	int fadeOutVolume;
};

#endif
