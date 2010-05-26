/*
 *  ofxSoundPlayerEngine.h
 *  openFrameworks
 *
 *  Created by Mehmet Akten on 13/08/2009.
 *  Copyright 2009 MSA Visuals Ltd.. All rights reserved.
 *  Modified by Pierre Proske on 20/11/2009
 *
 */

// This is the base class / interface which all implementations and the wrapper complies to

#pragma once

#include "ofMain.h"

/* Uncomment one of these to decide on which soundsystem to use */
//#define USE_FMOD
#define USE_OPENAL


class ofxSoundPlayerEngine {
public:

	ofxSoundPlayerEngine() {
	    isStreaming     = false;
        bMultiPlay      = false;
		bLoop 			= false;
		bLoadedOk 		= false;
		bPaused 		= false;
		pan 			= 0.0f;
		volume 			= 1.0f;
		internalFreq 	= 0;
		speed 			= 1;
		length          = 0;
		duration        = 0;
		channelCount    = 0;
	};

	virtual ~ofxSoundPlayerEngine() {}

	virtual bool loadSound(string fileName, bool stream = false) = 0;
	virtual void unloadSound() = 0;
	virtual void play() = 0;
	virtual void stop() = 0;

	virtual void setVolume(float vol) = 0;
	virtual void setPan(float pan) = 0;
	virtual void setSpeed(float spd) = 0;
	virtual void setPaused(bool bP) = 0;
	virtual void setLoop(bool bLp) = 0;
	virtual void setMultiPlay(bool bMp) = 0;
	virtual void setPosition(float pct) = 0; // 0 = start, 1 = end;

	virtual float getPosition() = 0;
	virtual bool getIsPlaying() = 0;
	virtual float getSpeed() = 0;
	virtual float getPan() = 0;
	virtual bool getLoop() = 0;
	virtual void assignEffect(unsigned int effectSlot, unsigned int slotID) = 0;
	virtual void removeEffect(unsigned int slotID) = 0;

	bool isStreaming;
	bool bMultiPlay;
	bool bLoop;
	bool bLoadedOk;
	bool bPaused;
	float pan; // -1 to 1
	float volume; // 0 - 1
	float internalFreq;
	float speed; // -n to n, 1 = normal, -1 backwards
	unsigned int length; // in samples;
	float duration; //in seconds
	int channelCount; // 1 or 2

};
