////////////////////////////////////////////////////////////
//
// ofxOpenALPlayer
// Copyright (C) 2009 Pierre Proske (pierre@digitalstar.net)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
#pragma once

#include "ofxSoundPlayer.h"

#include "Listener.h"
#include "Sound.h"
#include "SoundStereo.h"
#include "SoundBuffer.h"
#include "Music.h"
#include "Effect.h"

using namespace ofxOpenALPlayer;


class ofxSoundPlayerOpenAL : public ofxSoundPlayerEngine {

public:

	ofxSoundPlayerOpenAL();
	virtual ~ofxSoundPlayerOpenAL() {};

// ----------------------------------------------------------------------------
// Player functions from ofSoundPlayerEngine:
// ----------------------------------------------------------------------------
	bool    loadSound(string fileName, bool stream = false);
	void    unloadSound();
	void    play();
	void    stop();

	void    setVolume(float vol);
	void    setPan(float vol);
	void    setSpeed(float spd);
	void    setPaused(bool bP);
	void    setLoop(bool bLp);
	void    setMultiPlay(bool bMp);
	void    setPosition(float pct); // 0 = start, 1 = end;

	float   getPosition();
	bool    getIsPlaying();
	float   getSpeed();
	float   getPan();
	bool    getLoop();

// ----------------------------------------------------------------------------
// OpenAL implementation specific:
// ----------------------------------------------------------------------------

    void assignEffect(unsigned int uiEffectSlot, unsigned int uiSlotID);
    void removeEffect(unsigned int slotID);

    SoundBuffer* buffer; // buffer containing loaded samples for sounds/streams
    Sound* sound;  // mono sound sample
    SoundStereo* stereoSound; // stereo sound sample
    Music* stream; // audio stream
//    Sound* sound_left;  // stereo right channel
//    Sound* sound_right; // stereo left channel
    vector <Sound*>  multiSounds; // vector of copied sounds
    bool bIsStereo; // is this sound stereo or not (currently ignored for streams)


};

