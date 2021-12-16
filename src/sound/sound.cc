#include "sound.h"

#include "../engine/engine.h"

sound::SoundFileInstance::SoundFileInstance(string fileName, streampos position, size_t size) {
	this->fileName = fileName;
	this->position = position;
	this->size = size;

	this->file = ifstream(engine->getFilePrefix() + "out.carton");
	this->file.seekg(this->position);
}

sound::SoundFile::SoundFile(string fileName, streampos position, size_t size) {
	this->fileName = fileName;
	this->position = position;
	this->size = size;
}

sound::SoundFile::~SoundFile() {
	
}

void sound::SoundFile::play() {
	SoundFileInstance instance(this->fileName, this->position, this->size);
	OggVorbis_File file;
	if(ov_open_callbacks(&instance, &file, NULL, 0, OV_CALLBACKS_IFSTREAM) < 0) {
		printf("Could not read sound '%s'\n", this->fileName.c_str());
		exit(1);
	}
}
