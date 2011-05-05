#include "game.h"

MusicPlayer::MusicPlayer() {
	currentMusic = NULL;
	currentMusicPlayer = this;
	currentCollection = NULL;
	volume = 64;

	musicEffect = -1;
	endTime = 0;
	startTime = 0;
	initialParam = 0;
	prevVolume = -1;
	effectState = 0;
	initialTime = 0;
	isMusicEffectsOn = false;
}

MusicPlayer::~MusicPlayer() {
}

int MusicPlayer::loadSongs(const string &songFile) {
	dout << "--- MusicPlayer::loadSongs" << endl;
	FILE *fin = fopen(songFile.c_str(), "r");
	if(!fin) {
		dout << "ERROR- Could not open music file: " << songFile << endl;
		exit(1);
		//		dout << "--- End MusicPlayer::loadSongs" << endl << endl;
	}
	SongCollection *s;

	while(IsNextString(fin, "music")) {
		s = new SongCollection;
		s->id = LoadInt(fin);
		s->nextSongMode = LoadInt(fin);
		s->volume = LoadInt(fin);
		while(!IsNextString(fin, "end_music")) {
			string file = LoadString(fin);
			if(file.size() == 0) {
				dout << "ERROR- while reading file: " << songFile << endl;
				dout << "--- End MusicPlayer::loadSongs" << endl << endl;
				delete s;
				fclose(fin);
				return 1;
			}
			FILE *musicFile = fopen(file.c_str(), "r");
			if(!musicFile) {
				dout << "ERROR- Could not open: " << file << " (skipping)" << endl;
			} else {
				fclose(musicFile);
				s->songFiles.push_back(file);
				dout << " music file: " << file << " loaded." << endl;
			}
		}
		if(s->songFiles.size() == 0) {
			dout << "ERROR- SongCollection: " << s->id << " contains 0 songs (not adding)" << endl;
			delete s;
		} else {
			allSongs.insert(make_pair<int, SongCollection*>(s->id, s));
		}
	}
	fclose(fin);
	Mix_HookMusicFinished(musicFinishedHandler);
	dout << "--- End MusicPlayer::loadSongs" << endl << endl;
	return 0;
}
int MusicPlayer::playMusic(int id, int resumeMode, bool shouldSetVolume) {
	if(currentMusic) {
		if(currentCollection->id == id) {
			if(resumeMode) {
				return 0;
			}
		}
	}
	stopMusic();
	currentCollection = NULL;
	SongCollection *s;
	map<int, SongCollection*>::iterator i = allSongs.find(id);
	if(i == allSongs.end()) {
		return 1;
	}
	s = i->second;
	if(s->songFiles.size() == 0) {
		return 2;
	}
	currentCollection = s;
	currentSongFile = s->songFiles.end();

	if(playNext(shouldSetVolume)) {
		currentCollection = NULL;
		return 3;
	}
	return 0;
}
int MusicPlayer::resumeMusic() {
	if(currentMusic) {
		if(!Mix_PlayingMusic()) {
			Mix_PlayMusic(currentMusic, 0);
		}
	}
	return 0;
}
int MusicPlayer::pauseMusic() {
	if(currentMusic) {
		if(Mix_PlayingMusic()) {
//			Mix_FadeOutMusic(250);
			Mix_PauseMusic();
		}
	}
	return 0;
}
int MusicPlayer::stopMusic() {
	if(currentMusic) {
		Mix_HaltMusic();
		Mix_FreeMusic(currentMusic);
		currentMusic = NULL;
	}
	return 0;
}
bool MusicPlayer::isPlaying() {
	return (Mix_PlayingMusic() > 0);
}

int MusicPlayer::setVolume(int new_volume) {
	int musVol = 127;
	if(currentCollection) {
		musVol = currentCollection->volume;
	}
	Mix_VolumeMusic((new_volume * musVol) / 128);
//	dout << "SETVOLUME: " << new_volume << " to mixer: " << ((new_volume * musVol) / 128) << endl;

	volume = new_volume;
	return 0;
}

void MusicPlayer::musicFinishedHandler() {
	currentMusicPlayer->playNext();
}
int MusicPlayer::playNext(bool shouldSetVolume) {
	stopMusic();

	if(!currentCollection) {
		dout << "ERRROR- Can't play next song as there is no song collection\n" << endl;
		return 1;
	}

	vector<string>::iterator i = currentSongFile;

	switch(currentCollection->nextSongMode) {
	case MUSIC_MODE_NEXT:
		{
			if(i == currentCollection->songFiles.end()) {
				i = currentCollection->songFiles.begin();
			} else {
				i++;
				if(i == currentCollection->songFiles.end()) {
					i = currentCollection->songFiles.begin();
				}
			}
			break;
		}
	case MUSIC_MODE_RANDOM:
		{
			int idx = rand() % currentCollection->songFiles.size();
			i = currentCollection->songFiles.begin();
			for(int tmp = 0; tmp < idx; tmp++) i++;
			break;
		}
	case MUSIC_MODE_STOP:
		{
			if(i == currentCollection->songFiles.end()) {
				i = currentCollection->songFiles.begin();
				break;
			}
			return 0;
		}
	case MUSIC_MODE_RANDOM_STOP:
		{
			if(i == currentCollection->songFiles.end()) {
				int idx = rand() % currentCollection->songFiles.size();
				i = currentCollection->songFiles.begin();
				for(int tmp = 0; tmp < idx; tmp++) i++;
				break;
			}
			return 0;
		}
	}

	if(i == currentCollection->songFiles.end()) {
		currentCollection = NULL;
		return 1;
	}

	string newFile = i->c_str();
	currentSongFile = i;
	currentMusic =  Mix_LoadMUS(newFile.c_str());
	if(!currentMusic) {
		dout << "ERROR- Could not load music file: " << newFile << endl;
		currentCollection = NULL;
		return 3;
	}
	dout << "Music: playing '" << *i << "'" << endl;
	Mix_PlayMusic(currentMusic, 0);
	if(shouldSetVolume) {
		setVolume(volume);
	}
	return 0;
}

int MusicPlayer::saveMusic(FILE *fout) {
	saveInt(fout, volume);
	saveInt(fout, isPlaying());
	dout << "saveMusic: " << endl;
	dout << "    is playing: " << (int)isPlaying() << endl;
	if(currentCollection) {
		dout << "    collection: " << currentCollection->id << endl;
		saveInt(fout, currentCollection->id);
	} else {
		dout << "    collection not present" << endl;
		saveInt(fout, -1);
	}

	return 0;
}
int MusicPlayer::loadMusic(FILE *fin) {
	int volume = readInt(fin);
	setVolume(volume);
	int shouldPlay = readInt(fin);
	int musicId = readInt(fin);
	dout << "loadMusic: " << endl;
	dout << "    is playing: " << shouldPlay << endl;
	dout << "    collection: " << musicId << endl;

	if(musicId != -1) {
		playMusic(musicId, 0);
	}

	if(!shouldPlay) {
		pauseMusic();
	}
	return 0;
}

bool MusicPlayer::updateSpecialEffect() {
	switch(musicEffect) {
	case MUSIC_FADE_OUT_AND_PAUSE:
		{
			isMusicEffectsOn = false;
			break;
		}
	case MUSIC_RESUME_AND_FADE_IN:
		{
			isMusicEffectsOn = false;
			break;
		}
	case MUSIC_FADE_TO_VOLUME:
		{
			if(!isPlaying()) {
				musicEffect = -1;
				isMusicEffectsOn = false;
				break;
			}
			if(endTime < current_time) {
				setVolume(initialParam);
				musicEffect = -1;
				isMusicEffectsOn = false;
				break;
			}
			double total = endTime - startTime;
			double current = current_time - startTime;
			int newVolume = (int) (((current / total) * (initialParam - startVolume)) + startVolume);
			if(newVolume != prevVolume) {
				setVolume(newVolume);
				prevVolume = newVolume;
			}
			break;
		}
	case MUSIC_CROSSFADE_SONGS:
		{
			switch(effectState) {
			case 0:
				{
					// Fade out.
					if(!isPlaying()) {
						musicEffect = -1;
						isMusicEffectsOn = false;
						break;
					}
					
					if(endTime < current_time) {
						effectState = 1;
						break;
					}
					double diff = current_time - startTime;
					int newVolume = (int)(startVolume - ((diff / initialTime) * startVolume));
					if(newVolume != prevVolume) {
						setVolume(newVolume);
						prevVolume = newVolume;
					}
					break;
				}
			case 1:
				{
					// Play new music.
					playMusic(initialParam, 0, false);
					setVolume(0);
					effectState = 2;
					startTime = current_time;
					endTime = current_time + initialTime;
					break;
				}
			case 2:
				{
					// Fade in
					if(!isPlaying()) {
						musicEffect = -1;
						isMusicEffectsOn = false;
						break;
					}
					
					if(endTime < current_time) {
						setVolume(fadeOutVolume);
						isMusicEffectsOn = false;
						break;
					}
					double diff = current_time - startTime;
					int newVolume = (int)((diff / initialTime) * fadeOutVolume);
					if(newVolume != prevVolume) {
						setVolume(newVolume);
						prevVolume = newVolume;
					}
					break;
				}
			default:
				{
					dout << "ERROR- MUSIC_CROSSFADE_SONGS unknown effect state: " << effectState << endl;
					exit(1);
					break;
				}
			}
			break;
		}
	case -1:
		{
			isMusicEffectsOn = false;
			break;
		}
	default:
		{
			dout << "ERROR- Unknown special music effect needs updated: " << musicEffect << endl;
			exit(1);
			break;
		}
	}
	return isMusicEffectsOn;
}

int MusicPlayer::doSpecialEffect(int effect, int totalTime, int param) {

	switch(effect) {
	case MUSIC_FADE_OUT_AND_PAUSE:
		{
			break;
		}
	case MUSIC_RESUME_AND_FADE_IN:
		{
			break;
		}
	case MUSIC_FADE_TO_VOLUME:
		{
			// Param is desired volume

			if(!isPlaying()) {
				musicEffect = -1;
				dout << "WARNING: Could not start music effect MUSIC_FADE_TO_VOLUME as there is no music playing" << endl;
				break;
			}
			totalTime /= 2; // As there is fade-in and fade-out
			musicEffect = effect;
			startTime = current_time;
			endTime = current_time + totalTime;
			initialParam = param;
			prevVolume = volume;
			startVolume = volume;
			break;
		}
	case MUSIC_CROSSFADE_SONGS:
		{
			// Param is next music collection id
			if(musicEffect == MUSIC_CROSSFADE_SONGS) {
				// Fade out value remains unchanged.
			} else if (musicEffect == MUSIC_FADE_TO_VOLUME) {
				fadeOutVolume = initialParam;
			} else {
				fadeOutVolume = volume;
			}
			dout << "Fade out to: " << fadeOutVolume << endl;
			musicEffect = effect;
			startTime = current_time;
			endTime = current_time + totalTime;
			initialParam = param;
			initialTime = totalTime;
			prevVolume = volume;
			startVolume = volume;
			effectState = 0;

			if(!isPlaying()) {
				dout << "WARNING: MUSIC_CROSSFADE_SONGS called, but there is no music playing" << endl;
				effectState = 1;
				if(startVolume == 0) {
					dout << "WARNING: startValue == 0, made to 127" << endl;
					startVolume = 127;
				}
			}
			break;
		}
	default:
		{
			dout << "ERROR- Unknown special music effect requested: " << effect << endl;
			exit(1);
			break;
		}
	}
	isMusicEffectsOn = true;
	return 0;
}

MusicPlayer *MusicPlayer::currentMusicPlayer = NULL;

