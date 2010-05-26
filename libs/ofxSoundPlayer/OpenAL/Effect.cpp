#include "Effect.h"
#include "AudioDevice.h"


ALvoid (AL_APIENTRY *p_alGenFilters)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *p_alDeleteFilters)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *p_alFilteri)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *p_alGenEffects)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *p_alGenAuxiliaryEffectSlots)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *p_alDeleteAuxiliaryEffectSlots)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *p_alAuxiliaryEffectSloti)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *p_alAuxiliaryEffectSlotf)(ALuint,ALenum,ALfloat);
ALvoid (AL_APIENTRY *p_alDeleteEffects)(ALsizei,ALuint*);
ALvoid (AL_APIENTRY *p_alEffecti)(ALuint,ALenum,ALint);
ALvoid (AL_APIENTRY *p_alEffectf)(ALuint,ALenum,ALfloat);

namespace ofxOpenALPlayer
{

int Effect::numSlots = -1;

Effect::Effect() : sends(0), bEFXSupported(false), uiEffectSlot(0), uiEffect(0), bEffectCreated(false)
{

    ALCdevice* device = AudioDevice::GetInstance().GetDevice();

    if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == ALC_FALSE)
    {
        ofLog(OF_LOG_ERROR,"EFX extention is not found");
    }
    else if (alcIsExtensionPresent(device, "ALC_EXT_EFX") ==ALC_TRUE)
    {
        ofLog(OF_LOG_NOTICE,"EFX Extension found!");
    }
    ALCheckError(__FILE__, __LINE__);

    ALCheck(alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &sends));
    //printf("Max auxiliary sends: %d\n", (int)sends);

    p_alGenFilters = (ALvoid (*)(ALsizei,ALuint*)) alGetProcAddress("alGenFilters");
    p_alDeleteFilters = (ALvoid (*)(ALsizei,ALuint*)) alGetProcAddress("alDeleteFilters");
    p_alFilteri = (ALvoid (*)(ALuint,ALenum,ALint)) alGetProcAddress("alFilteri");
    p_alGenEffects = (ALvoid (*)(ALsizei,ALuint*)) alGetProcAddress("alGenEffects");
    p_alGenAuxiliaryEffectSlots = (ALvoid (*)(ALsizei,ALuint*)) alGetProcAddress("alGenAuxiliaryEffectSlots");
    p_alDeleteAuxiliaryEffectSlots = (ALvoid (*)(ALsizei,ALuint*)) alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    p_alAuxiliaryEffectSloti = (ALvoid (*)(ALuint,ALenum,ALint)) alGetProcAddress("alAuxiliaryEffectSloti");
    p_alAuxiliaryEffectSlotf = (ALvoid (*)(ALuint,ALenum,ALfloat)) alGetProcAddress("alAuxiliaryEffectSlotf");
    p_alDeleteEffects = (ALvoid (*)(ALsizei,ALuint*)) alGetProcAddress("alDeleteEffects");
    p_alEffecti = (ALvoid (*)(ALuint,ALenum, ALint)) alGetProcAddress("alEffecti");
    p_alEffectf = (ALvoid (*)(ALuint,ALenum, ALfloat)) alGetProcAddress("alEffectf");
    ALCheckError(__FILE__, __LINE__);

    if(!p_alGenFilters || !p_alDeleteFilters || !p_alFilteri || !p_alGenEffects || !p_alGenAuxiliaryEffectSlots || !p_alDeleteAuxiliaryEffectSlots ||
       !p_alAuxiliaryEffectSloti || !p_alAuxiliaryEffectSlotf || !p_alDeleteEffects || !p_alEffecti || !p_alEffectf) {

        ofLog(OF_LOG_ERROR, "Missing EFX functions, unable to create Effects!");
        return;
    }
    else {
        bEFXSupported = true;
        numSlots++;
        uiSlotID = numSlots;
    }

}

Effect::~Effect()
{
    // Load NULL Effect into Effect Slot
    if(uiEffectSlot) {
        ALCheck(p_alAuxiliaryEffectSloti(uiEffectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL));
    }

    // Delete Effect
    if(uiEffect) {
        ALCheck(p_alDeleteEffects(1, &uiEffect));
    }

    // Delete Auxiliary Effect Slot
    if(uiEffectSlot) {
        cout << "delete Aux Effect" << endl;
        ALCheck(p_alDeleteAuxiliaryEffectSlots(1, &uiEffectSlot));
    }

}

ALuint Effect::getEffect()
{
    return uiEffectSlot;
}

ALuint Effect::getEffectID()
{
    return uiSlotID;
}

void Effect::setEffectParameter(ALenum eEffectType, float value)
{
    ALCheck(p_alEffectf(uiEffect, eEffectType, value));

    // Load Effect into Auxiliary Effect Slot
    ALCheck(p_alAuxiliaryEffectSloti(uiEffectSlot, AL_EFFECTSLOT_EFFECT, uiEffect));
}

void Effect::setEffectGain(float value)
{
    // Set effect gain
    ALCheck(p_alAuxiliaryEffectSlotf(uiEffectSlot, AL_EFFECTSLOT_GAIN, value));
}

void Effect::addEffect(ALenum eEffectType)
{

	if (bEFXSupported)
	{
		// The EFX Extension includes support for global effects, such as Reverb.  To use a global effect,
		// you need to create an Auxiliary Effect Slot to store the Effect ...
		if (createAuxEffectSlot(&uiEffectSlot))
		{
			// Once we have an Auxiliary Effect Slot, we can generate an Effect Object, set its Type
			// and Parameter Values, and then load the Effect into the Auxiliary Effect Slot ...
			if (createEffect(&uiEffect, eEffectType))
			{
				bEffectCreated = true;
			}
			else
			{
				ofLog(OF_LOG_ERROR,"Failed to create an Effect");
			}
		}
		else
		{
			ofLog(OF_LOG_ERROR,"Failed to generate an Auxilary Effect Slot");
		}

        // Load Effect into Auxiliary Effect Slot
        ALCheck(p_alAuxiliaryEffectSloti(uiEffectSlot, AL_EFFECTSLOT_EFFECT, uiEffect));
	}

}

bool Effect::createAuxEffectSlot(ALuint *puiAuxEffectSlot)
{
	bool bReturn = false;

	// Clear AL Error state
	alGetError();

	// Generate an Auxiliary Effect Slot
	ALCheck(p_alGenAuxiliaryEffectSlots(1, puiAuxEffectSlot));
	if (alGetError() == AL_NO_ERROR)
		bReturn = AL_TRUE;

	return bReturn;
}

bool Effect::createEffect(ALuint *puiEffect, ALenum eEffectType)
{
	bool bReturn = false;

	if (puiEffect)
	{
		// Clear AL Error State
		alGetError();

		// Generate an Effect
		p_alGenEffects(1, puiEffect);
		if (alGetError() == AL_NO_ERROR)
		{
			// Set the Effect Type
			ALCheck(p_alEffecti(*puiEffect, AL_EFFECT_TYPE, eEffectType));
			if (alGetError() == AL_NO_ERROR)
				bReturn = AL_TRUE;
			else
				ALCheck(p_alDeleteEffects(1, puiEffect));
		}
	}

	return bReturn;
}

} // namespace ofxOpenALPlayer
