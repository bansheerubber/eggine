#include "engine.h"

#include <AL/al.h>
#include <AL/alc.h>

#include "../engine/engine.h"
#include "sound.h"

sound::Engine::Engine() {
	
}

void sound::Engine::initialize() {
	this->device = alcOpenDevice(nullptr);
	this->context = alcCreateContext(this->device, nullptr);
	alcMakeContextCurrent(this->context);

	// do special stuff for extensions meant for streaming
	auto resources = engine->manager->carton->database.get()->equals("extension", ".ogg")->exec();
	for(size_t i = 0; i < resources.head; i++) {
		string fileName = resources[i]->getMetadata("fileName");
		engine->soundEngine.addSound(new sound::SoundFile(
			nullptr,
			nullptr,
			fileName,
			engine->manager->carton->getFileLocation(fileName),
			engine->manager->carton->getFileSize(fileName)
		));
	}
}

void sound::Engine::setPosition(glm::vec3 position) {
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void sound::Engine::addSound(sound::SoundFile* file) {
	this->sounds.push_back(file);
	this->fileToSound[file->fileName] = file;
}

void sound::Engine::playSoundByFileName(string fileName) {
	this->fileToSound[fileName]->play();
}
