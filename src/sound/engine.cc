#include "engine.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "../engine/engine.h"
#include "sound.h"
#include "soundCollection.h"

#ifdef __switch__
Mutex FinishedThreadsWrite;
#else
std::mutex FinishedThreadsWrite;
#endif

sound::Engine::Engine() {
	
}

void sound::Engine::initialize() {
	this->device = alcOpenDevice(nullptr);
	this->context = alcCreateContext(this->device, nullptr);
	alcMakeContextCurrent(this->context);

	// do special stuff for extensions meant for streaming
	auto resources = engine->manager->carton->database.get()->equals("extension", ".ogg")->exec();
	for(uint64_t i = 0; i < resources.head; i++) {
		std::string fileName = resources[i]->getMetadata("fileName");
		engine->soundEngine.addSound(new sound::Sound(engine->manager, resources[i]));
	}

	auto resources2 = engine->manager->carton->database.get()->equals("extension", ".wav")->exec();
	for(uint64_t i = 0; i < resources2.head; i++) {
		std::string fileName = resources2[i]->getMetadata("fileName");
		engine->soundEngine.addSound(new sound::Sound(engine->manager, resources2[i]));
	}

	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".sound")->exec());
}

void sound::Engine::tick() {
	uint64_t size = this->finishedThreads.size();
	for(unsigned int i = 0; i < size; i++) {
		SoundThreadContext* context = this->finishedThreads[i];
		
		#ifdef __switch__
		threadClose(context->thread);
		delete context->thread;
		#endif

		delete context;
	}

	#ifdef __switch__
	mutexLock(&FinishedThreadsWrite);
	for(uint64_t i = 0; i < size; i++) {
		this->finishedThreads.pop_front();
	}
	mutexUnlock(&FinishedThreadsWrite);
	#else
	FinishedThreadsWrite.lock();
	for(uint64_t i = 0; i < size; i++) {
		this->finishedThreads.pop_front();
	}
	FinishedThreadsWrite.unlock();
	#endif
}

void sound::Engine::setPosition(glm::vec3 position) {
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void sound::Engine::addSound(sound::Sound* file) {
	this->sounds.push_back(file);
	this->fileToSound[file->fileName] = file;
}

void sound::Engine::addCollection(class SoundCollection* collection) {
	this->soundCollections.push_back(collection);
	this->nameToCollection[collection->name] = collection;
}

void sound::Engine::playSoundByFileName(std::string fileName) {
	this->fileToSound[fileName]->play();
}

void sound::Engine::playSoundByCollectionName(std::string collectionName) {
	if(this->nameToCollection.find(collectionName) != this->nameToCollection.end()) {
		this->nameToCollection[collectionName]->play();
	}
}

void sound::Engine::finishThread(struct SoundThreadContext* context) {
	#ifdef __switch__
	mutexLock(&FinishedThreadsWrite);
	this->finishedThreads.push_back(context);
	mutexUnlock(&FinishedThreadsWrite);
	#else
	FinishedThreadsWrite.lock();
	this->finishedThreads.push_back(context);
	FinishedThreadsWrite.unlock();
	#endif
}

void es::defineSoundEngine() {
	esEntryType playSoundArguments[1] = { ES_ENTRY_STRING };
	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, &es::playSound, "playSound", 1, playSoundArguments);
}

esEntryPtr es::playSound(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		engine->soundEngine.playSoundByCollectionName(std::string(args[0].stringData));
	}
	return nullptr;
}
