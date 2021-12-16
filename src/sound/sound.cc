#include "sound.h"

#include <iostream>
#include <thread>

#include "../engine/engine.h"
#include "soundReader.h"

sound::Sound::Sound(
	resources::ResourceManager* manager,
	carton::Metadata* metadata
) : resources::ResourceObject(manager, metadata) {
	this->fileName = metadata->getMetadata("fileName");
	this->position = engine->manager->carton->getFileLocation(this->fileName);
	this->size = engine->manager->carton->getFileSize(this->fileName);

	SoundFileType type = OGG_FILE;
	if(this->fileName.find("wav") != string::npos) {
		type = WAV_FILE;
	}

	SoundReader reader(this->position, this->size, type);

	// read data into the buffers
	char* buffer = new char[SOUND_BUFFER_SIZE];

	int currentSection = 0;
	for(size_t i = 0; i < SOUND_BUFFER_CIRCULAR_COUNT; i++) {
		size_t result = reader.readIntoBuffer(buffer, SOUND_BUFFER_SIZE);
		this->buffers[i].setData(buffer, result, reader.getSampleRate(), MONO_16_BIT);

		if(result != SOUND_BUFFER_SIZE) {
			this->bufferCount = i + 1;
			this->filled = false;
			break;
		}
	}

	delete[] buffer;
}

void sound::Sound::play() {
	thread t(&Sound::_play, this);
	t.detach();
}

void sound::Sound::_play() {
	int bufferIndex = -SOUND_BUFFER_COUNT;
	ALuint source;
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, 1);
	alSourcef(source, AL_GAIN, 1.0f);
	alSource3f(source, AL_POSITION, 0, 0, 0);
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	alSourcei(source, AL_LOOPING, AL_FALSE);
	
	if(!this->filled) {
		ALuint buffers[this->bufferCount];
		for(unsigned int i = 0; i < this->bufferCount; i++) {
			buffers[i] = this->buffers[i].bufferId; // load up pre-loaded buffers
		}
		
		alSourceQueueBuffers(source, this->bufferCount, buffers);
		alSourcePlay(source);

		ALint state = AL_PLAYING;
		while(state == AL_PLAYING) {
			alGetSourcei(source, AL_SOURCE_STATE, &state); // update the state
			this_thread::sleep_for(chrono::milliseconds(SOUND_THREAD_WAIT));
		}

		alDeleteSources(1, &source); // cleanup the source
		return;
	}

	SoundFileType type = OGG_FILE;
	if(this->fileName.find("wav") != string::npos) {
		type = WAV_FILE;
	}

	SoundReader reader(this->position, this->size, type);

	// read data into the buffers
	char* buffer = new char[SOUND_BUFFER_SIZE];

	reader.seek(SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT / 2.0); // seek some buffers worth into the .ogg stream

	int currentSection = 0;

	ALuint buffers[SOUND_BUFFER_CIRCULAR_COUNT];
	for(unsigned int i = 0; i < SOUND_BUFFER_CIRCULAR_COUNT; i++) {
		buffers[i] = this->buffers[i].bufferId; // load up pre-loaded buffers
	}

	alSourceQueueBuffers(source, SOUND_BUFFER_CIRCULAR_COUNT, buffers);
	alSourcePlay(source);

	ALint state = AL_PLAYING;
	bool done = false;
	while(state == AL_PLAYING) {
		alGetSourcei(source, AL_SOURCE_STATE, &state); // update the state
		
		ALint buffersProcessed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed); // figure out if we need to update buffers

		if(buffersProcessed == 0) {
			this_thread::sleep_for(chrono::milliseconds(SOUND_THREAD_WAIT));
			continue;
		}

		// go through the queue and update buffers
		ALuint poppedBuffer;
		if(done) {
			alSourceUnqueueBuffers(source, 1, &poppedBuffer);
			break;
		}
		
		alSourceUnqueueBuffers(source, 1, &poppedBuffer);

		if(bufferIndex < 0) {
			poppedBuffer = this->buffers[bufferIndex + SOUND_BUFFER_COUNT].bufferId;
			bufferIndex++;
		}
		
		size_t result = reader.readIntoBuffer(buffer, SOUND_BUFFER_SIZE);
		if(result < 0) { // error
			break;
		}
		else if(result != SOUND_BUFFER_SIZE) {
			done = true;
		}

		alBufferData(poppedBuffer, AL_FORMAT_MONO16, (void*)buffer, result, reader.getSampleRate());
		alSourceQueueBuffers(source, 1, &poppedBuffer);
		buffersProcessed--;
	}

	alDeleteSources(1, &source);
	delete[] buffer;
}
