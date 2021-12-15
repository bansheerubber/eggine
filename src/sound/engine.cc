#include "engine.h"

#include <AL/al.h>
#include <AL/alc.h>

sound::Engine::Engine() {
	this->device = alcOpenDevice(nullptr);
	this->context = alcCreateContext(this->device, nullptr);
}

void sound::Engine::setPosition(glm::vec3 position) {
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}
