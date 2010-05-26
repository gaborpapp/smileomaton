////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2009 Laurent Gomila (laurent.gom@gmail.com)
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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "AudioDevice.h"
#include "AudioResource.h"
#include "Listener.h"
#include <algorithm>
#include <iostream>


namespace ofxOpenALPlayer
{

////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////
AudioDevice* AudioDevice::ourInstance;


// note: MAX_SOURCES is how many source you want
// to preload.  should keep it below 32
#define MAX_SOURCES 300;

void preloadSources()
{
        // lazy init of my data structure

	// we want to allocate all the sources we will need up front
	int sourceCount = MAX_SOURCES;
	int sourceIndex;
	ALuint sourceID;
	// build a bunch of sources and load them into our array.
	for (sourceIndex = 0; sourceIndex < sourceCount; sourceIndex++)
	{
	    cout << "sources = " << sourceIndex << endl;
		alGenSources(1, &sourceID);
        int err=0;
        if((err=alGetError()) != AL_FALSE) {
            cout <<"Couldn't generate source name, maxmimum numbers of sources: " << sourceIndex << endl;
            break;
        }

	}

//	for(int i = 0; i <= sourceIndex; i++)
//	{
//	    alDeleteSources();
//	}
}


////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
AudioDevice::AudioDevice() :
myRefCount(0)
{
    float listenerDepth = 0.1;

    // Create the device
    myDevice = alcOpenDevice(NULL);
    PrintOpenALInfo();
    if (myDevice)
    {
        // Create the context
        myContext = alcCreateContext(myDevice, NULL);

        if (myContext)
        {
            // Set the context as the current one (we'll only need one)
            alcMakeContextCurrent(myContext);

            // Initialize the listener, located at the origin and looking along the Z axis
            Listener::SetPosition(0,0,listenerDepth);
            Listener::SetTarget(0,0,-listenerDepth);

            //preloadSources();
            //exit(1);
        }
        else
        {
            std::cerr << "Failed to create the audio context" << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to open the audio device" << std::endl;
    }


}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
AudioDevice::~AudioDevice()
{

    // Destroy the context
    alcMakeContextCurrent(NULL);
    if (myContext) {
        alcDestroyContext(myContext);
        myContext = NULL;
    }

    // Destroy the device
    if (myDevice) {
        alcCloseDevice(myDevice);
        myDevice = NULL;
    }
}


////////////////////////////////////////////////////////////
/// Get the unique instance of the class
////////////////////////////////////////////////////////////
AudioDevice& AudioDevice::GetInstance()
{
    // Create the audio device if it doesn't exist
    if (!ourInstance)
        ourInstance = new AudioDevice;

    return *ourInstance;
}


////////////////////////////////////////////////////////////
/// Add a reference to the audio device
////////////////////////////////////////////////////////////
void AudioDevice::AddReference()
{
    // Create the audio device if it doesn't exist
    if (!ourInstance)
        ourInstance = new AudioDevice;

    // Increase the references count
    ourInstance->myRefCount++;
    cout << "ref count = " << ourInstance->myRefCount << endl;
}


////////////////////////////////////////////////////////////
/// Remove a reference to the audio device
////////////////////////////////////////////////////////////
void AudioDevice::RemoveReference()
{
    // Decrease the references count
    ourInstance->myRefCount--;
    cout << "ref count = " << ourInstance->myRefCount << endl;

    // Destroy the audio device if the references count reaches 0
    if (ourInstance->myRefCount == 0)
    {
        Listener::RemoveAllSounds();
        Listener::RemoveAllStreams();
        delete ourInstance;
        ourInstance = NULL;
    }
}


////////////////////////////////////////////////////////////
/// Get the OpenAL audio device
////////////////////////////////////////////////////////////
ALCdevice* AudioDevice::GetDevice() const
{
    return myDevice;
}


////////////////////////////////////////////////////////////
/// Get the OpenAL format that matches the given number of channels
////////////////////////////////////////////////////////////
ALenum AudioDevice::GetFormatFromChannelsCount(unsigned int ChannelsCount) const
{
    // Find the good format according to the number of channels
    switch (ChannelsCount)
    {
        case 1 : return AL_FORMAT_MONO16;
        case 2 : return AL_FORMAT_STEREO16;
        case 4 : return alGetEnumValue("AL_FORMAT_QUAD16");
        case 6 : return alGetEnumValue("AL_FORMAT_51CHN16");
        case 7 : return alGetEnumValue("AL_FORMAT_61CHN16");
        case 8 : return alGetEnumValue("AL_FORMAT_71CHN16");
    }

    return 0;
}


////////////////////////////////////////////////////////////
/// Print some interesting info about OpenAL
////////////////////////////////////////////////////////////
void AudioDevice::PrintOpenALInfo()
{
//	if (alGetString(AL_VERSION))
//		cout << "OpenAL version: "    << alGetString(AL_VERSION)    << endl;
//	if (alGetString(AL_RENDERER))
//		cout << "OpenAL renderer: "   << alGetString(AL_RENDERER)   << endl;
//	if (alGetString(AL_VENDOR))
//		cout << "OpenAL vendor: "     << alGetString(AL_VENDOR)     << endl;
//	if (alGetString(AL_EXTENSIONS))
//		cout << "OpenAL extensions: " << alGetString(AL_EXTENSIONS) << endl;

	// Enumerate OpenAL devices
	if (alcIsExtensionPresent (NULL, (const ALCchar *) "ALC_ENUMERATION_EXT") == AL_TRUE)
	{
		const char *s = (const char *) alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		while (*s != '\0')
		{
			cout << "OpenAL available device: " << s << endl;
			while (*s++ != '\0');
		}
	}
	else
	{
		cout << "OpenAL device enumeration isn't available." << endl;
	}

	// Print default device name
	cout << "OpenAL default device: "
	     << (const char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER)
		 << endl;

	// Print current device name
	if (myDevice)
	{
		cout << "OpenAL current device: "
			 << (const char *)alcGetString(myDevice, ALC_DEVICE_SPECIFIER)
			 << endl;
	}
}

} // namespace ofxOpenALPlayer
