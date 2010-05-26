#include "ofxSoundPlayer.h"
#include "ofUtils.h"



// now, the individual sound player:
//------------------------------------------------------------
ofxSoundPlayer::ofxSoundPlayer(){
	_engine = ofxNewSoundPlayerEngine();
}

//------------------------------------------------------------
ofxSoundPlayer::~ofxSoundPlayer(){
	unloadSound();
	if(_engine) delete _engine;
}

//------------------------------------------------------------
bool ofxSoundPlayer::loadSound(string fileName, bool stream){
	return _engine->loadSound(fileName, stream);
}

//------------------------------------------------------------
void ofxSoundPlayer::unloadSound(){
	_engine->unloadSound();
}

//------------------------------------------------------------
bool ofxSoundPlayer::getIsPlaying(){
	return _engine->getIsPlaying();
}

//------------------------------------------------------------
float ofxSoundPlayer::getSpeed(){
	return _engine->getSpeed();
}

//------------------------------------------------------------
float ofxSoundPlayer::getPan(){
	return _engine->getPan();
}

//------------------------------------------------------------
void ofxSoundPlayer::setVolume(float vol){
	_engine->setVolume(vol);
}

//------------------------------------------------------------
void ofxSoundPlayer::setPosition(float pct){
	_engine->setPosition(pct);
}

//------------------------------------------------------------
float ofxSoundPlayer::getPosition(){
	return _engine->getPosition();
}

//------------------------------------------------------------
bool ofxSoundPlayer::getLoop(){
	return _engine->getLoop();
}

//------------------------------------------------------------
void ofxSoundPlayer::setPan(float p){
	_engine->setPan(p);
}


//------------------------------------------------------------
void ofxSoundPlayer::setPaused(bool bP){
	_engine->setPaused(bP);

}


//------------------------------------------------------------
void ofxSoundPlayer::setSpeed(float spd){
	_engine->setSpeed(spd);
}


//------------------------------------------------------------
void ofxSoundPlayer::setLoop(bool bLp){
	_engine->setLoop(bLp);
}

// ----------------------------------------------------------------------------
void ofxSoundPlayer::setMultiPlay(bool bMp){
	_engine->setMultiPlay(bMp);
}

// ----------------------------------------------------------------------------
void ofxSoundPlayer::play(){
	_engine->play();
}

// ----------------------------------------------------------------------------
void ofxSoundPlayer::stop(){
	_engine->stop();
}

// ----------------------------------------------------------------------------
void ofxSoundPlayer::assignEffect(unsigned int effectSlot, unsigned int slotID){
	_engine->assignEffect(effectSlot, slotID);
}

// ----------------------------------------------------------------------------
void ofxSoundPlayer::removeEffect(unsigned int slotID){
	_engine->removeEffect(slotID);
}

