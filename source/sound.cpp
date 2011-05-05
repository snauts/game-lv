#include "game.h" 
#include "sound.h"

/*
 * Can use MIX_CHANNELS because Mix_AllocateChannels is not used.
 */
static unsigned loop_info[MIX_CHANNELS];

int Mix_GetChannelState(int channel, int *playing, int *loopsLeft) {
	if (Mix_Playing(channel)) {
		*playing = 1;
		*loopsLeft = loop_info[channel];
		return channel;
	} else {
		*playing = 0;
		*loopsLeft = -1;
		return -1;
	}
}

int PlayChannel(int channel, Mix_Chunk *chunk, int loops) {
    loop_info[channel] = loops;
    Mix_PlayChannel(channel, chunk, loops);
}

Mix_Chunk *getNextSoundCollectionChunk(SoundCollection *sound, bool shouldGetNext = true);
string getNextSoundCollectionFile(SoundCollection *sound);

Mix_Chunk *loadSoundFile(string &file, int type);

static map<string, LoadedChunk*> loadedChunks;

SoundCollection *loadSoundCollection(const string &file, int type) {
	if(file.length() == 0) {
		dout << " loadSoundCollection::Loading empty sound file..." << endl;
		return NULL;
	}
	FILE *fin = fopen(file.c_str(), "r");
	if(!fin) {
		return NULL;
	}
	if(!IsNextString(fin, "sound")) {
		return NULL;
	}
	SoundCollection *c = new SoundCollection;
	c->id = LoadInt(fin);
	c->priority = LoadInt(fin);
	c->volume = LoadInt(fin);
	c->playCount = LoadInt(fin) - 1;
	c->nextSoundMode = LoadInt(fin);
	c->type = type;
	while(!IsNextString(fin, "end_sound")) {
		string soundFile = LoadString(fin);
		convertPathInPlace(soundFile);
		if(soundFile.size() == 0) {
			dout << "ERROR- loadSoundCollection::while reading file: " << file << endl;
			delete c;
			fclose(fin);
			return NULL;
		}
		c->soundFiles.push_back(soundFile);
	}
	fclose(fin);
	dout << " loadSoundCollection:: sound " << file << " has " << c->soundFiles.size() << " files" << endl;
	return c;
}

void loadScriptSounds(map<int, SoundCollection*> &scriptSound) {
	dout << "--- loadScriptSound --- " << endl;
	
	vector<string> fileNames;
	loadListFile("scriptSounds.lst", fileNames);
	
	char lineBuf[256];
	SoundCollection *c;
	for(int currFile = 0; currFile < fileNames.size(); currFile++) {
		strcpy(lineBuf, fileNames[currFile].c_str());
		dout << "  loading script sound file: " << lineBuf << endl;
		c = loadSoundCollection(lineBuf, SOUND_TYPE_SCRIPT);
		if(c) {
			map<int, SoundCollection*>::iterator i = scriptSounds.find(c->id);
			if(i != scriptSounds.end()) {
				dout << "ERROR- script sound with id:" << c->id << "already exists" << endl;
				exit(1);
			} else {
				scriptSound.insert(make_pair<int, SoundCollection*>(c->id, c));
			}
		} else {
			dout << "WARNING- could not load sound file: '" << lineBuf << "'" << endl;
		}

	}
	dout << "--- END loadScriptSound --- " << endl;
}

string getNextSoundCollectionFile(SoundCollection *sound) {
	string ret;

	if(sound->soundFiles.size() == 0) {
		return ret;
	}
	
	switch(sound->nextSoundMode) {
	case SOUND_MODE_NEXT:
		{
			sound->currentFileIdx++;
			if(sound->currentFileIdx >= sound->soundFiles.size()) {
				sound->currentFileIdx = 0;
			}
			ret = sound->soundFiles[sound->currentFileIdx];
			break;
		}
	case SOUND_MODE_RANDOM:
		{
			sound->currentFileIdx = rand() % sound->soundFiles.size();
			ret = sound->soundFiles[sound->currentFileIdx];
			break;
		}
	default:
		{
			dout << "ERROR- Unknown sound play mode..." << endl;
			break;
		}
	}
	return ret;
}

Mix_Chunk *getNextSoundCollectionChunk(SoundCollection *sound, bool shouldGetNext) {

	Mix_Chunk *tmp = NULL;
	string newFile;
	if(shouldGetNext || (sound->currentSoundFile.length() == 0)) {
		newFile = getNextSoundCollectionFile(sound);
	} else {
		newFile = sound->currentSoundFile;
	}

	if(newFile.length() > 0) {
		tmp = loadSoundFile(newFile, sound->type);
	}
	sound->currentSoundFile = newFile;
	return tmp;
}

SoundPlayer::SoundPlayer() {
}

SoundPlayer::~SoundPlayer() {
}


unsigned SoundPlayer::playSound(int channel, SoundCollection *sound) {
	if(!sound) {
		dout << " Wanted to play NULL sound on " << channel << " channel" << endl;
		return 0;
	}
	Mix_Chunk *chunk = getNextSoundCollectionChunk(sound);
	if(chunk) {
		Mix_Volume(channel, sound->volume);
		PlayChannel(channel, chunk, sound->playCount);
	} else {
		Mix_HaltChannel(channel);
		return 1;
	}
	return 0;
}

unsigned SoundPlayer::playUiSound(SoundCollection *sound) {
	stopUiSound();
	playSound(SOUND_CHANNEL_UI, sound);
	return 0;
}

unsigned SoundPlayer::stopUiSound() {
	Mix_HaltChannel(SOUND_CHANNEL_UI);
	return 0;
}

unsigned SoundPlayer::playSceneSound(SoundCollection *sound) {
	stopSceneSound();
	playSound(SOUND_CHANNEL_SCENE, sound);
	return 0;
}

unsigned SoundPlayer::stopSceneSound() {
	Mix_HaltChannel(SOUND_CHANNEL_SCENE);
	return 0;
}

unsigned SoundPlayer::playCharacterSound(SoundCollection *sound) {
	stopCharacterSound();
	playSound(SOUND_CHANNEL_CHARACTER, sound);
	return 0;
}

unsigned SoundPlayer::stopCharacterSound() {
	Mix_HaltChannel(SOUND_CHANNEL_CHARACTER);
	return 0;
}


unsigned SoundPlayer::pauseAll() {
	Mix_Pause(-1);
	return 0;
}

unsigned SoundPlayer::resumeAll() {
	Mix_Resume(-1);
	return 0;
}

unsigned SoundPlayer::getHotspotSoundChannel(int hotpotId) {
	unsigned a;
	map<unsigned, HotspotSoundInfo>::iterator i;
	for(a = 0; a < TOTAL_HOTSPOT_CHANNELS; a++) {
		i = hotspotSounds.find(a + SOUND_CHANNEL_HOTSPOTS);
		if(i == hotspotSounds.end()) {
			return (a  + SOUND_CHANNEL_HOTSPOTS);
		}
	}
	return 0;
}
unsigned SoundPlayer::freeHotspotChannel(int priority) {
	map<unsigned, HotspotSoundInfo>::iterator i = hotspotSounds.begin();
	HotspotSoundInfo info;
	unsigned channel = 0;
	while(i != hotspotSounds.end()) {
		info = i->second;
		if(priority > info.priority) {
			channel = i->first;
			Mix_HaltChannel(channel);
			hotspotSounds.erase(i);
			break;
		}
		i++;
	}
	return channel;
}
unsigned SoundPlayer::freeNonPlayingHotspotChannels() {
	map<unsigned, HotspotSoundInfo>::iterator i = hotspotSounds.begin();
	map<unsigned, HotspotSoundInfo>::iterator tmp;

	while(i != hotspotSounds.end()) {
		if(!Mix_Playing(i->first)) {
			// This channel is not playing anymore.
			tmp = i;
			i++;
			hotspotSounds.erase(tmp);
		} else {
			i++;
		}
	}
	return 0;
}
unsigned SoundPlayer::stopAllHotspotSound() {
	hotspotSounds.clear();
	unsigned a;
	for(a = 0; a < TOTAL_HOTSPOT_CHANNELS; a++) {
		Mix_HaltChannel(a + SOUND_CHANNEL_HOTSPOTS);
	}
	return 0;
}
unsigned SoundPlayer::playHotspotSound(int hotspotId, SoundCollection *sound) {
	if(!sound) {
		stopHotspotSound(hotspotId);
		return 0;
	}
	unsigned channel = 0;
	if(!(channel = getHotspotChannel(hotspotId))) {
		freeNonPlayingHotspotChannels();
		if(!(channel = getHotspotSoundChannel(hotspotId))) {
			if(!(channel = freeHotspotChannel(sound->priority))) {
				return 1;
			}
		}
	}


	if(channel) {
		map<unsigned, HotspotSoundInfo>::iterator i = hotspotSounds.find(channel);
		if(i != hotspotSounds.end()) {
			hotspotSounds.erase(i);
		}
		Mix_HaltChannel(channel);
		HotspotSoundInfo info;
		info.channel = channel;
		info.hotspotId = hotspotId;
		info.priority = sound->priority;
		hotspotSounds.insert(make_pair<unsigned, HotspotSoundInfo>(channel, info));
		playSound(channel, sound);
	} else {
		return 1;
	}
	return 0;
}

unsigned SoundPlayer::getHotspotChannel(int hotspotId) {
	map<unsigned, HotspotSoundInfo>::iterator i = hotspotSounds.begin(); 
	while(i != hotspotSounds.end()) {
		if(i->second.hotspotId == hotspotId) {
			return i->first;
		}
		i++;
	}
	return 0;
}
unsigned SoundPlayer::stopHotspotSound(int hotspotId) {
	unsigned channel;
	if((channel = getHotspotChannel(hotspotId))) {
		map<unsigned, HotspotSoundInfo>::iterator i = hotspotSounds.find(channel); 
		Mix_HaltChannel(channel);
		hotspotSounds.erase(i);
	}
	return 0;
}

unsigned SoundPlayer::playScriptSound(int channel, SoundCollection *sound) {
	if(channel >= TOTAL_SCRIPT_CHANNELS) {
		dout << "ERROR- channel number exceeds TOTAL_SCRIPT_CHANNELS: " << TOTAL_SCRIPT_CHANNELS << endl;
		exit(1);
//		return 1;
	}
	map<int, SoundCollection*>::iterator i = playedScriptSounds.find(SOUND_CHANNEL_SCRIPT + channel);
	if(i != playedScriptSounds.end()) {
		playedScriptSounds.erase(i);
	}
	playedScriptSounds.insert(make_pair<int, SoundCollection*>(SOUND_CHANNEL_SCRIPT + channel, sound));
	stopScriptSound(channel);
	playSound(SOUND_CHANNEL_SCRIPT + channel, sound);
	return 0;
}
unsigned SoundPlayer::isPlayingScriptSound(int channel) {
	if(channel >= TOTAL_SCRIPT_CHANNELS) {
		dout << "ERROR- channel number exceeds TOTAL_SCRIPT_CHANNELS: " << TOTAL_SCRIPT_CHANNELS << endl;
		exit(1);
//		return 1;
	}
	return Mix_Playing(SOUND_CHANNEL_SCRIPT + channel);
}
unsigned SoundPlayer::stopScriptSound(int channel) {
	if(channel >= TOTAL_SCRIPT_CHANNELS) {
		dout << "ERROR- channel number exceeds TOTAL_SCRIPT_CHANNELS: " << TOTAL_SCRIPT_CHANNELS << endl;
		exit(1);
//		return 1;
	}
	Mix_HaltChannel(SOUND_CHANNEL_SCRIPT + channel);
	return 0;
}

unsigned SoundPlayer::playDialogSound(string &file) {
	stopDialogSound();
	playSoundCustom(SOUND_CHANNEL_SPEECH, file, SOUND_TYPE_DIALOG);
	return 0;
}
unsigned SoundPlayer::isDialogPlaying() {
	return Mix_Playing(SOUND_CHANNEL_SPEECH);
}
unsigned SoundPlayer::stopDialogSound() {
	Mix_HaltChannel(SOUND_CHANNEL_SPEECH);
	return 0;
}
unsigned SoundPlayer::stopAll() {
	stopUiSound();
	stopSceneSound();
	stopCharacterSound();
	stopAllHotspotSound();
	for(unsigned a = 0; a < TOTAL_SCRIPT_CHANNELS; a++) {
		stopScriptSound(a);
	}
	playedScriptSounds.clear();
	stopDialogSound();
	return 0;
}

int SoundPlayer::saveSceneSoundState(FILE *fout) {
	saveSoundState(fout, SOUND_CHANNEL_SCENE);
	return 0;
}

int SoundPlayer::loadSceneSoundState(FILE *fin, SoundCollection* sound) {
	loadSoundState(fin, sound, SOUND_CHANNEL_SCENE);
	return 0;
}

int SoundPlayer::saveCharacterSoundState(FILE *fout) {
	saveSoundState(fout, SOUND_CHANNEL_CHARACTER);
	return 0;
}

int SoundPlayer::loadCharacterSoundState(FILE *fin, SoundCollection* sound) {
	loadSoundState(fin, sound, SOUND_CHANNEL_CHARACTER);
	return 0;
}

int SoundPlayer::saveDialogSoundState(FILE *fout) {
	saveSoundState(fout, SOUND_CHANNEL_SPEECH);
	return 0;
}
int SoundPlayer::loadDialogSoundState(FILE *fin, string &file) {
	loadSoundStateCustom(fin, file, SOUND_CHANNEL_SPEECH, SOUND_TYPE_DIALOG);
	return 0;
}


int SoundPlayer::saveSoundState(FILE *fout, int channel) {
	int currLoops, playing;
	if(Mix_GetChannelState(channel, &playing, &currLoops) != channel) {
		saveInt(fout, -1);
	} else {
		saveInt(fout, playing);
		saveInt(fout, currLoops);
	}
	dout << " saveSoundState, channel: " 
		<< channel	<< " playing: " << playing
		<< ", loops: " << currLoops << endl;
	return 0;
}

int SoundPlayer::loadSoundState(FILE *fin, SoundCollection *sound, int channel) {
	int playing, loops = 0;
	playing = readInt(fin);
	if(playing != -1) {
		loops = readInt(fin);
		Mix_Chunk *tmp = loadSoundFile(sound->currentSoundFile, sound->type);
		if(!tmp) {
			dout << " loadSoundState- sound->currentSound == NULL, can't resume sound" << endl;
		} else {
			if (loops < 0) loops = -1;
			Mix_Volume(channel, sound->volume); //lai nebuutu troksnis loadojot (barvins)
			PlayChannel(channel, tmp, loops);
			Mix_Pause(channel);
		}
	}
	dout << " loadSoundState, channel: " 
		<< channel	<< " playing: " << playing
		<< ", loops: " << loops << endl;
	return 0;
}

int SoundPlayer::saveHotspotSoundState(FILE *fout, int hotspotId) {
	map<unsigned, HotspotSoundInfo>::iterator i = hotspotSounds.begin(); 
	while(i != hotspotSounds.end()) {
		if(i->second.hotspotId == hotspotId) {
			break;
		}
		i++;
	}
	if(i != hotspotSounds.end()) {
		saveInt(fout, 1);
		saveInt(fout, hotspotId);
		saveInt(fout, i->second.priority);
		saveInt(fout, i->second.channel);
		saveSoundState(fout, i->second.channel);
	} else {
		saveInt(fout, 0);
	}
	return 0;
}
int SoundPlayer::loadHotspotSoundState(FILE *fin, int hotspotId, SoundCollection* sound) {
	int shouldLoad = readInt(fin);
	if(shouldLoad) {
		int hId = readInt(fin);
		if(hId != hotspotId) {
			dout << "Error in loading hotspot sound, loadHotspotSoundState, id: " 
				<< hotspotId << endl;
		}
		int priority = readInt(fin);
		int channel = readInt(fin);
		loadSoundState(fin, sound, channel);
		HotspotSoundInfo i;
		i.hotspotId = hotspotId;
		i.channel = channel;
		i.priority = priority;
		hotspotSounds.insert(make_pair<unsigned, HotspotSoundInfo>(channel, i));
	}
	return 0;
}

map<int, SoundCollection*> SoundPlayer::getCurrentlyPlayingScriptSounds() {
	map<int, SoundCollection*>::iterator i = playedScriptSounds.begin();
	map<int, SoundCollection*>::iterator tmp;
	while(i != playedScriptSounds.end()) {
		tmp = i;
		i++;
		if(!Mix_Playing(tmp->first)) {
			dout << "   Removed stopped sound: " << tmp->second->id 
				<< " on channel:" << (tmp->first) - SOUND_CHANNEL_SCRIPT << endl;
			playedScriptSounds.erase(tmp);
		} else {
			dout << "   Playing script sound: " << tmp->second->id
				<< " on channel: " << tmp->first - SOUND_CHANNEL_SCRIPT << endl;
		}
	}
	return playedScriptSounds;
}
int SoundPlayer::saveScriptSoundChannel(FILE *fout, int channel) {
	saveSoundState(fout, channel);
	return 0;
}
int SoundPlayer::loadScriptSoundChannel(FILE *fin, int channel, SoundCollection *sound) {
	loadSoundState(fin, sound, channel);
	return 0;
}


int saveSoundCollectionState(FILE *fout, SoundCollection* sound) {
	saveString(fout, sound->currentSoundFile);
	saveInt(fout, sound->currentFileIdx);
	return 0;
}

int loadSoundCollectionState(FILE *fin, SoundCollection *sound) {
	sound->currentSoundFile = readString(fin);
	sound->currentFileIdx = readInt(fin);
	return 0;	
}


int SoundPlayer::skipSound(int skipTime) {
	return 0;
}

Mix_Chunk *loadSoundFile(string &file, int type) {
	dout << "    loadSoundFile called, file: " << file << ", type: " << type << endl;
	if(!file.length()) {
		dout << "      wanted to load empty file" << endl;
		return NULL;
	}

	map<string, LoadedChunk*>::iterator i = loadedChunks.find(file);
	LoadedChunk *t = NULL;
	if(i == loadedChunks.end()) {
		dout << "      file not found in cache" << endl;
		t = new LoadedChunk;
		t->file = file;
		t->type = type;
		t->chunk = Mix_LoadWAV(convertPath(file).c_str());
		if(t->chunk) {
			dout << "      file: " << file << " loaded successfully" << endl;
		} else {
			dout << "ERROR- failed to load sound file: " << file << endl;
			exit(1);
		}
		loadedChunks.insert(make_pair<string, LoadedChunk*>(file, t));
		dout << endl << "--- Sound files cached: ---" << endl;
		i = loadedChunks.begin();
		while(i != loadedChunks.end()) {
			dout << "    " << i->first << ", type: " << hex << i->second->type << dec << endl;
			i++;
		}
		dout << "--- Totaly: " << loadedChunks.size() << " file(s) cached ---" << endl << endl;
	} else {
		t = i->second;
		dout << "      file found in cache" << endl;
		if((t->type & type) != type) {
			dout << "      file type differs, adding this type" << endl;
			t->type |= type;
		}
	}
	return t->chunk;
}

bool isChunkPlaying(Mix_Chunk *chunk) {
	return false;
}

int SoundPlayer::freeSoundData(int type) {
	map<string, LoadedChunk*>::iterator i = loadedChunks.begin();
	int count = 0;
	dout << "--- Free sound data (type: " << hex << type << dec <<  ") ---" << endl;
	while(i != loadedChunks.end()) {
		LoadedChunk *c = i->second;
		dout << "    file: " << c->file << ", type: " << hex << c->type  << dec << endl;
		if(c->type & type) {
			if((c->type & ~type) == 0) {
				if(!isChunkPlaying(c->chunk)) {
					map<string, LoadedChunk*>::iterator tmp = i;
					i++;
					loadedChunks.erase(tmp);
					Mix_FreeChunk(c->chunk);
					delete c;
					dout << "      freed" << endl;
					count++;
					continue;
				}
				
			} else {
				dout << "      partial match, removed only type status" << endl;
				c->type &= ~type;
			}
		} else {
			dout << "      no match" << endl;
		}
		i++;
	}
	dout << "--- Free sound data completed, freed: " << count << " file(s)" << endl;
	return 0;
}

int SoundPlayer::playSoundCustom(int channel, string &file, int type) {
	Mix_Chunk *c = loadSoundFile(file, type);
	if(c) {
		Mix_Volume(channel, 127);
		PlayChannel(channel, c, 0);
	} else {
		Mix_HaltChannel(channel);
		return 1;
	}
	return 0;
}

int SoundPlayer::loadSoundStateCustom(FILE *fin, string &file, int channel, int type) {
	int playing, loops;
	playing = readInt(fin);
	if(playing != -1) {
		loops = readInt(fin);
		Mix_Chunk *tmp = loadSoundFile(file, type);
		if(!tmp) {
			dout << " loadSoundStateCustom- loadSoundFile returned NULL, can't resume sound" << endl;
		} else {
			Mix_Volume(channel, 127);  //lai nebuutu troksnis loadojot (barvins)
			PlayChannel(channel, tmp, loops);
			Mix_Pause(channel);
		}
	}
	dout << " loadSoundStateCustom, channel: " 
		<< channel	<< " playing: " << playing
		<< ", loops: " << loops << endl;
	return 0;
}
