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

#ifndef OFX_SOUNDPLAYER_AUDIODEVICE_H
#define OFX_SOUNDPLAYER_AUDIODEVICE_H

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "OpenAL.h"
#include <set>
#include <string>


namespace ofxOpenALPlayer
{

class AudioResource;


////////////////////////////////////////////////////////////
/// AudioDevice is the high-level wrapper around the audio API,
/// it manages creation and destruction of the audio device and context
/// and stores the device capabilities
////////////////////////////////////////////////////////////
class AudioDevice
{
public :

    ////////////////////////////////////////////////////////////
    /// Get the unique instance of the class
    ///
    /// \return Unique instance of the class
    ///
    ////////////////////////////////////////////////////////////
    static AudioDevice& GetInstance();

    ////////////////////////////////////////////////////////////
    /// Add a reference to the audio device
    ///
    ////////////////////////////////////////////////////////////
    static void AddReference();

    ////////////////////////////////////////////////////////////
    /// Remove a reference to the audio device
    ///
    ////////////////////////////////////////////////////////////
    static void RemoveReference();

    ////////////////////////////////////////////////////////////
    /// Get the OpenAL audio device
    ///
    /// \return OpenAL device (cannot be NULL)
    ///
    ////////////////////////////////////////////////////////////
    ALCdevice* GetDevice() const;

    ////////////////////////////////////////////////////////////
    /// Get the OpenAL format that matches the given number of channels
    ///
    /// \param ChannelsCount : Number of channels
    ///
    /// \return OpenAL device (cannot be NULL)
    ///
    ////////////////////////////////////////////////////////////
    ALenum GetFormatFromChannelsCount(unsigned int ChannelsCount) const;

    ////////////////////////////////////////////////////////////
    /// Print OpenAL details
    ///
    ////////////////////////////////////////////////////////////
    void PrintOpenALInfo();


private :

    ////////////////////////////////////////////////////////////
    /// Default constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioDevice();

    ////////////////////////////////////////////////////////////
    /// Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~AudioDevice();

    ////////////////////////////////////////////////////////////
    // Static member data
    ////////////////////////////////////////////////////////////
    static AudioDevice* ourInstance; ///< Unique instance of the audio device

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ALCdevice*   myDevice;   ///< Audio device
    ALCcontext*  myContext;  ///< Audio context
    unsigned int myRefCount; ///< References count
};


} // namespace ofxOpenALPlayer


#endif // OFX_SOUNDPLAYER_AUDIODEVICE_H
