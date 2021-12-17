#include "engine.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "../engine/engine.h"
#include "sound.h"

#ifdef __switch__
Mutex FinishedThreadsWrite;
#else
mutex FinishedThreadsWrite;
#endif

sound::Engine::Engine() {
	
}

void sound::Engine::initialize() {
	this->device = alcOpenDevice(nullptr);
	this->context = alcCreateContext(this->device, nullptr);
	alcMakeContextCurrent(this->context);

	// do special stuff for extensions meant for streaming
	auto resources = engine->manager->carton->database.get()->equals("extension", ".ogg")->exec(true);
	for(size_t i = 0; i < resources.head; i++) {
		string fileName = resources[i]->getMetadata("fileName");
		engine->soundEngine.addSound(new sound::Sound(engine->manager, resources[i]));
	}

	auto resources2 = engine->manager->carton->database.get()->equals("extension", ".wav")->exec(true);
	for(size_t i = 0; i < resources2.head; i++) {
		string fileName = resources2[i]->getMetadata("fileName");
		engine->soundEngine.addSound(new sound::Sound(engine->manager, resources2[i]));
	}
}

void sound::Engine::tick() {
	size_t size = this->finishedThreads.size();
	for(unsigned int i = 0; i < size; i++) {
		SoundThreadContext* context = this->finishedThreads[i];
		
		#ifdef __switch__
		threadClose(context->thread);
		delete context->thread;
		#endif

		printf("closed thread\n");

		delete context;
	}

	#ifdef __switch__
	mutexLock(&FinishedThreadsWrite);
	for(size_t i = 0; i < size; i++) {
		this->finishedThreads.pop_front();
	}
	mutexUnlock(&FinishedThreadsWrite);
	#else
	FinishedThreadsWrite.lock();
	for(size_t i = 0; i < size; i++) {
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

void sound::Engine::playSoundByFileName(string fileName) {
	this->fileToSound[fileName]->play();
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
