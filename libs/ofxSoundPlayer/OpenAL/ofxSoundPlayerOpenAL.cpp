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

#include "ofxSoundPlayerOpenAL.h"
#include "ofUtils.h"


// ----------------------------------------------------------------------------
// these don't need to be implemented in the OpenAL sound engine
// ----------------------------------------------------------------------------
//void ofxSoundInitialize() {}
//void ofxSoundClose(){}

#ifdef USE_OPENAL
// ----------------------------------------------------------------------------
// these are global functions, that affect every sound / channel:
// ----------------------------------------------------------------------------
ofxSoundPlayerEngine* ofxNewSoundPlayerEngine()
{
	return new ofxSoundPlayerOpenAL;
}

// ----------------------------------------------------------------------------
void ofxSoundStopAll()
{
    Listener::StopAllSounds();
    Listener::StopAllStreams();
}

// ----------------------------------------------------------------------------
void ofxSoundSetVolume(float vol)
{

    vol = ofClamp(vol,0,1);
    Listener::SetGlobalVolume(vol*100);
}

// ----------------------------------------------------------------------------
float * ofxSoundGetSpectrum(int nBands)
{
	ofLog(OF_LOG_WARNING,"ofxSoundGetSpectrum() not yet implemented in the OpenAL player");
	return NULL;
}

#endif

// ----------------------------------------------------------------------------
// Individual sound player functions
// ----------------------------------------------------------------------------
ofxSoundPlayerOpenAL::ofxSoundPlayerOpenAL()
: buffer(0), sound(0), stereoSound(0), stream(0), bIsStereo(false) //, sound_left(0), sound_right(0)
{

}

// ----------------------------------------------------------------------------
bool ofxSoundPlayerOpenAL::loadSound(string fileName, bool bIsStream)
{
    bLoadedOk = false;
	fileName = ofToDataPath(fileName);

	unloadSound();

    if(bIsStream)
    {
        stream = new Music();

        // Load a stream
        if (!stream->OpenFromFile(fileName)) {
            ofLog(OF_LOG_ERROR,"ofxSoundPlayer : Failed to load stream %s",fileName.c_str());
            return false;
        }

        internalFreq = stream->GetSampleRate();
        duration = stream->GetDuration();
        length = static_cast<unsigned int> (internalFreq * duration);
        channelCount = stream->GetChannelsCount();

        isStreaming = true;
        bLoadedOk = true;
    }
    else
    {
        buffer = new SoundBuffer();

        // Load a sound buffer from a file
        if (!buffer->LoadFromFile(fileName)) {
            ofLog(OF_LOG_ERROR,"ofxSoundPlayer : Failed to load sound %s",fileName.c_str());
            return false;
        }

        internalFreq = buffer->GetSampleRate();
        duration = buffer->GetDuration();
        channelCount = buffer->GetChannelsCount();
        length = buffer->GetSamplesCount(); // length in samples


        if(channelCount == 2)
        {
            bIsStereo = true;

            stereoSound = new SoundStereo();
            stereoSound->SetBuffer(*buffer);

            // extract stereo channels
//            const Int16* buf = buffer->GetSamples();
//            int newLength = length/2;
//            Int16* buf1 = new Int16[newLength];
//            Int16* buf2 = new Int16[newLength];
//
//            for(int i = 0;i < newLength;i++)
//            {
//                buf1[i] = *buf++;
//                buf2[i] = *buf++;
//            }
//
//            SoundBuffer* buffer_left = new SoundBuffer();
//            SoundBuffer* buffer_right = new SoundBuffer();
//            buffer_left->LoadFromSamples(buf1,newLength,channelCount/2,internalFreq);
//            buffer_right->LoadFromSamples(buf2,newLength,channelCount/2,internalFreq);
//
//            // Create a stereo sound instance
//            sound_left = new Sound();
//            sound_right = new Sound();
//            sound_left->SetBuffer(*buffer_left);
//            sound_right->SetBuffer(*buffer_right);
//
//            sound_left->SetPosition(-1,0,0);
//            sound_right->SetPosition(1,0,0);
//
//            delete [] buf1;
//            delete [] buf2;

            bLoadedOk = true;

        }
        else
        {
            // create a mono sound instance
            sound = new Sound();
            sound->SetBuffer(*buffer);
            bLoadedOk = true;
        }

    }

    return bLoadedOk;
}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::unloadSound()
{
    if (!bLoadedOk)
	{
	    return;
	}

    if(sound)
    {
        sound->Stop();
        delete sound;
        sound = NULL;
    }
//    if(sound_left)
//    {
//        sound_left->Stop();
//        delete sound_left;
//        sound_left = NULL;
//    }
//    if(sound_right)
//    {
//        sound_right->Stop();
//        delete sound_right;
//        sound_right = NULL;
//    }
    if(stereoSound)
    {
        stereoSound->Stop();
        delete stereoSound;
        stereoSound = NULL;
    }
    if(stream)
    {
        stream->Stop();
        delete stream;
        stream = NULL;
    }
    if(multiSounds.size() > 0)
    {
        vector<Sound*>::iterator itr;
        for(itr = multiSounds.begin(); itr < multiSounds.end();itr++)
        {
            (*itr)->Stop();
            delete (*itr);
        }
        multiSounds.clear();
    }
    if(buffer)
    {
        delete buffer;
        buffer = NULL;
    }
}

// ----------------------------------------------------------------------------
bool ofxSoundPlayerOpenAL::getIsPlaying()
{
    Sound::SoundStatus status;
	if (!bLoadedOk)
	{
	    ofLog(OF_LOG_WARNING,"ofxSoundPlayer : No sound or stream loaded.");
	    return false;
	}

    if(stream)
    {
        status = stream->GetStatus();
        if( status == Sound::Playing) {
            return true;
        }
    }
//    else if(sound_left && sound_right)
//    {
//        status = sound_left->GetStatus();
//        if( status == Sound::Playing) {
//            return true;
//        }
//    }
    else if(stereoSound)
    {
        status = stereoSound->GetStatus();
        if( status == Sound::Playing) {
            return true;
        }
    }
    else if(sound)
    {
        status = sound->GetStatus();
        if( status == Sound::Playing) {
            return true;
        }
    }

    return false;

}

// ----------------------------------------------------------------------------
float ofxSoundPlayerOpenAL::getSpeed()
{
    if(stream)
    {
        speed = stream->GetPitch();
    }
//    else if(sound_left && sound_right)
//    {
//        speed = sound_left->GetPitch();
//    }
    else if(stereoSound)
    {
        speed = sound->GetPitch();
    }
    else if(sound)
    {
        speed = sound->GetPitch();
    }
    else {
        ofLog(OF_LOG_WARNING,"ofxSoundPlayer : No sound or stream loaded.");
    }

    return speed;
}

// ----------------------------------------------------------------------------
float ofxSoundPlayerOpenAL::getPan()
{
    return pan;
}

// ----------------------------------------------------------------------------
bool ofxSoundPlayerOpenAL::getLoop()
{
    if(stream)
    {
        return stream->GetLoop();
    }
//    else if(sound_left && sound_right)
//    {
//        return sound_left->GetLoop();
//    }
    else if(stereoSound)
    {
        return stereoSound->GetLoop();
    }
    else if(sound)
    {
        return sound->GetLoop();
    }
    else {
        ofLog(OF_LOG_WARNING,"ofxSoundPlayer : No sound or stream loaded.");
    }

    return false;
}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setVolume(float vol)
{
    volume = ofClamp(vol,0,1);

    if(stream)
    {
        stream->SetVolume(volume*100.0f);
    }
//    else if(sound_left && sound_right)
//    {
//        sound_left->SetVolume((1.0f - pan)*volume*100.0f);
//        sound_right->SetVolume(pan*volume*100.0f);
//    }
    else if(stereoSound)
    {
        stereoSound->SetVolume(volume*100.0f);
    }
    else if(sound)
    {
        sound->SetVolume(volume*100.0f);
    }


}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setPosition(float pct)
{
    pct = ofClamp(pct,0,1);
    if(sound)
    {
        sound->SetPlayingOffset(pct);
    }
    if(stereoSound)
    {
        stereoSound->SetPlayingOffset(pct);
    }
//    else if(sound_left && sound_right)
//    {
//        pct = ofClamp(pct,0,1);
//        sound_left->SetPlayingOffset(pct);
//        sound_right->SetPlayingOffset(pct);
//    }
    else if(stream)
    {
        ofLog(OF_LOG_WARNING,"Cannot set position of an OpenAL stream");
    }
}

// ----------------------------------------------------------------------------
float ofxSoundPlayerOpenAL::getPosition()
{
    if(stream)
    {
        return stream->GetPlayingOffset();
    }
//    else if(sound_left && sound_right)
//    {
//        return sound_left->GetPlayingOffset();
//    }
    else if(stereoSound)
    {
        return stereoSound->GetPlayingOffset();
    }
    else if(sound)
    {
        return sound->GetPlayingOffset();
    }
    else return 0;
}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setPan(float _pan)
{
    pan = ofClamp(_pan, -1, 1);
    //cout << "pan = " << pan << endl;
    //cout << "volume = " << volume << endl;

    if(stream)
    {
        stream->SetPosition(pan,0,0);
    }
//    else if(sound_left && sound_right)
//    {
//        float p = (pan + 1.0f)/2.0f;
//        sound_left->SetVolume((1.0f - p)*2*volume*100.0f);
//        sound_right->SetVolume(p*volume*2*100.0f);
//    }
    else if(stereoSound)
    {
        stereoSound->SetPan(pan);
    }
    else if(sound)
    {
        sound->SetPosition(pan,0,0);
    }
}


// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setPaused(bool bP)
{
    Sound::SoundStatus status;
    bPaused = bP;

    if(stream)
    {
        status = stream->GetStatus();
        if(bPaused) {
            stream->Pause();
        }
        else
        {
            if (status == Sound::Paused) {
                stream->Play();
            }
        }
    }
//    else if(sound_left && sound_right)
//    {
//        status = sound_left->GetStatus();
//        if(bPaused) {
//            sound_left->Pause();
//            sound_right->Pause();
//        }
//        else
//        {
//            if (status == Sound::Paused) {
//                sound_left->Play();
//                sound_right->Play();
//            }
//        }
//    }
    else if(stereoSound)
    {
        status = stereoSound->GetStatus();
        if(bPaused) {
            stereoSound->Pause();
        }
        else
        {
            if (status == Sound::Paused) {
                stereoSound->Play();
            }
        }
    }
    else if(sound)
    {
        status = sound->GetStatus();
        if(bPaused) {
            sound->Pause();
        }
        else
        {
            if (status == Sound::Paused) {
                sound->Play();
            }
        }
    }

}


// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setSpeed(float spd)
{
    spd = ofClamp(spd, 0, 128);
    if(spd < 0) {
        ofLog(OF_LOG_WARNING,"OpenAL cannot set negative speeds.");
    }

    if(stream)
    {
        stream->SetPitch(spd);
    }
//    else if(sound_left && sound_right)
//    {
//        sound_left->SetPitch(spd);
//        sound_right->SetPitch(spd);
//    }
    else if(stereoSound)
    {
        stereoSound->SetPitch(spd);
    }
    else if(sound)
    {
        sound->SetPitch(spd);
    }

}


// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setLoop(bool bLp)
{
    if(stream)
    {
        stream->SetLoop(bLp);
    }
//    else if(sound_left && sound_right)
//    {
//        sound_left->SetLoop(bLp);
//        sound_right->SetLoop(bLp);
//    }
    else if(stereoSound)
    {
        stereoSound->SetLoop(bLp);
    }
    else if(sound)
    {
        sound->SetLoop(bLp);
    }

}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::setMultiPlay(bool bMp)
{
    if(bIsStereo) {
        ofLog(OF_LOG_WARNING,"setMultiPlay() not yet implemented for stereo sounds in the OpenAL player");
    } else if (isStreaming) {
        ofLog(OF_LOG_WARNING,"setMultiPlay() does not function with streams");
    } else {
        bMultiPlay = bMp;
    }
}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::play()
{
    if(stream)
    {
        stream->Play();
    }
//    else if(sound_left && sound_right)
//    {
//        sound_left->Play();
//        sound_right->Play();
//    }
    else if(stereoSound)
    {
        stereoSound->Play();
    }
    else if(sound)
    {
        if(bMultiPlay)
        {
            bool hasPlayed = false;
            Sound::SoundStatus status = sound->GetStatus();
            if( status == Sound::Playing)
            {
                vector<Sound*>::iterator itr;
                for(itr = multiSounds.begin(); itr < multiSounds.end();itr++)
                {
                    status = (*itr)->GetStatus();
                    if(status != Sound::Playing)
                    {
                        (*itr)->Play();
                        hasPlayed = true;
                        itr = multiSounds.end();
                    }
                }
                if(!hasPlayed)
                {
                    multiSounds.push_back(new Sound(*sound));
                    multiSounds.back()->Play();
                    //cout << "multisounds : " << multiSounds.size() << endl;
                }

            }
            else
            {
                sound->Play();
            }
        }
        else
        {
            sound->Play();
        }
    }


}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::stop(){
    if(stream)
    {
        stream->Stop();
    }
//    else if(sound_left && sound_right)
//    {
//        sound_left->Stop();
//        sound_right->Stop();
//    }
    else if(stereoSound)
    {
        stereoSound->Stop();
    }
    else if(sound)
    {
        sound->Stop();
    }

}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::assignEffect(unsigned int effectSlot, unsigned int slotID)
{
    sound->AssignEffect(effectSlot,slotID);
}

// ----------------------------------------------------------------------------
void ofxSoundPlayerOpenAL::removeEffect(unsigned int slotID)
{
    sound->RemoveEffect(slotID);
}
