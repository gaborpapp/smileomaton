#pragma once

#include "ofxSoundPlayerEngine.h"


void ofxSoundStopAll();
void ofxSoundSetVolume(float vol);
float * ofxSoundGetSpectrum(int nBands); // max 512...

// MEMO: added these globals which would be defined in the implementation specific file
// PIERRE: need to remove these in favour of reference counting to init/delete the sound system
void ofxSoundInitialize();					// instead of initFmod();
void ofxSoundClose();						// instead of closeFmod();

ofxSoundPlayerEngine *ofxNewSoundPlayerEngine();		// returns a new implementation specific ofxSoundPlayerEngine*


class ofxSoundPlayer : public ofxSoundPlayerEngine
{
public:

	// from ofxSoundPlayerEngine:
	ofxSoundPlayer();
	virtual ~ofxSoundPlayer();

	bool loadSound(string fileName, bool stream = false);
	void unloadSound();
	void play();
	void stop();

	void setVolume(float vol);
	void setPan(float vol);
	void setSpeed(float spd);
	void setPaused(bool bP);
	void setLoop(bool bLp);
	void setMultiPlay(bool bMp);
	void setPosition(float pct); // 0 = start, 1 = end;

	float getPosition();
	bool getIsPlaying();
	float getSpeed();
	float getPan();
	bool getLoop();

	void assignEffect(unsigned int effectSlot, unsigned int slotID);
	void removeEffect(unsigned int slotID);


protected:
	ofxSoundPlayerEngine *_engine;
};
