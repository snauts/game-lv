#ifndef __SOUND_H__
#define __SOUND_H__

#include "game.h"

const int TOTAL_HOTSPOT_CHANNELS = 2;
const int TOTAL_SCRIPT_CHANNELS = 2;

const int SOUND_CHANNEL_CHARACTER = 0;
const int SOUND_CHANNEL_SCENE = 1;
const int SOUND_CHANNEL_SPEECH = 2;
const int SOUND_CHANNEL_UI = 3;
const int SOUND_CHANNEL_HOTSPOTS = 4;
const int SOUND_CHANNEL_SCRIPT = SOUND_CHANNEL_HOTSPOTS + TOTAL_HOTSPOT_CHANNELS;


const int SOUND_TYPE_UNKNOWN = 0x01;
const int SOUND_TYPE_GLOBAL = 0x02;
const int SOUND_TYPE_SCENE = 0x04;
const int SOUND_TYPE_HOTSPOT = 0x08;
const int SOUND_TYPE_CHARACTER = 0x10;
const int SOUND_TYPE_DIALOG = 0x20;
const int SOUND_TYPE_UI = 0x40;
const int SOUND_TYPE_SCRIPT = 0x80;

struct SoundCollection {
	int id;
	int priority; // the bigger this value is, the bigger the priority.
	int volume;
	int type;
	int playCount;
	int nextSoundMode;
	vector<string> soundFiles;

	int currentFileIdx; // Needed to allow several equal file names in sound collection.
	string currentSoundFile;

	SoundCollection() {
		id = -1;
		type = SOUND_TYPE_UNKNOWN;
		nextSoundMode = -1;
		priority = 0;
		currentFileIdx = -1;
		volume = MIX_MAX_VOLUME;
		playCount = 0;
	}
	~SoundCollection() {
	}
};

struct HotspotSoundInfo {
	unsigned channel;
	int hotspotId;
	int priority;
};

struct LoadedChunk {
	string file;
	Mix_Chunk *chunk;
	int type;
};

SoundCollection *loadSoundCollection(const string &file, int type);
void loadScriptSounds(map<int, SoundCollection*> &scriptSound);

int saveSoundCollectionState(FILE *fout, SoundCollection* sound);
int loadSoundCollectionState(FILE *fin, SoundCollection *sound);


class SoundPlayer {

	unsigned playSound(int channel, SoundCollection *sound);

	unsigned getHotspotSoundChannel(int hotpotId);
	unsigned freeHotspotChannel(int priority);
	unsigned freeNonPlayingHotspotChannels();
	unsigned getHotspotChannel(int hotspotId);

	int saveSoundState(FILE *fout, int channel);
	int loadSoundState(FILE *fin, SoundCollection *sound, int channel);
	int loadSoundStateCustom(FILE *fin, string &file, int channel, int type);

	// first is channel.
	map<unsigned, HotspotSoundInfo> hotspotSounds;
	map<int, SoundCollection*> playedScriptSounds;
	
public:
	SoundPlayer();
	~SoundPlayer();
	
	int skipSound(int skipTime);

	unsigned playUiSound(SoundCollection *sound);
	unsigned stopUiSound();

	unsigned playSceneSound(SoundCollection *sound);
	unsigned stopSceneSound();
	int saveSceneSoundState(FILE *fout);
	int loadSceneSoundState(FILE *fin, SoundCollection* sound);
	
	unsigned playCharacterSound(SoundCollection *sound);
	unsigned stopCharacterSound();
	int saveCharacterSoundState(FILE *fout);
	int loadCharacterSoundState(FILE *fin, SoundCollection* sound);
	
	unsigned stopAllHotspotSound(); // Must be called before new scene is started.
	unsigned playHotspotSound(int hotspotId, SoundCollection *sound);
	unsigned stopHotspotSound(int hotspotId);
	map<unsigned, HotspotSoundInfo> getCurrentlyPlayingHotspots() { return hotspotSounds; }
	int saveHotspotSoundState(FILE *fout, int hotspotId);
	int loadHotspotSoundState(FILE *fin, int hotspotId, SoundCollection* sound);

	unsigned playScriptSound(int channel, SoundCollection *sound);
	unsigned isPlayingScriptSound(int channel);
	unsigned stopScriptSound(int channel);
	map<int, SoundCollection*> getCurrentlyPlayingScriptSounds();
	int saveScriptSoundChannel(FILE *fout, int channel);
	int loadScriptSoundChannel(FILE *fin, int channel, SoundCollection *sound);

	unsigned playDialogSound(string &file);
	unsigned isDialogPlaying();
	unsigned stopDialogSound();
	int saveDialogSoundState(FILE *fout);
	int loadDialogSoundState(FILE *fin, string &file);

	int freeSoundData(int type);
	int playSoundCustom(int channel, string &file, int type);
	
	unsigned pauseAll();
	unsigned resumeAll();
	unsigned stopAll();

};

#endif
