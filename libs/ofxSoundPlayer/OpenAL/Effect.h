#ifndef OFX_SOUNDPLAYER_EFFECT_H
#define OFX_SOUNDPLAYER_EFFECT_H

#include "AudioResource.h"
#include "OpenAL.h"



namespace ofxOpenALPlayer
{


class Effect : public AudioResource
{
    public:
        Effect();
        virtual ~Effect();

        void addEffect(ALenum eEffectType);
        void setEffectParameter(ALenum eEffectType, float value);
        void setEffectGain(float value);
        ALuint getEffect();
        ALuint getEffectID();

    protected:
        bool createAuxEffectSlot(ALuint *puiAuxEffectSlot);
        bool createEffect(ALuint *puiEffect, ALenum eEffectType);

        ALint sends;
        bool bEFXSupported;
        ALuint	uiEffectSlot;
        ALuint  uiEffect;
        ALuint  uiSlotID;
        bool bEffectCreated;

    static int numSlots;
    private:
};


} // namespace ofxOpenALPlayer

#endif // OFX_SOUNDPLAYER_EFFECT_H
