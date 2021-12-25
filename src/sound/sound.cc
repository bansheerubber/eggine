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
	char* buffer = new char[SOUND_BUFFER_SIZE * reader.getChannels()];

	for(size_t i = 0; i < SOUND_BUFFER_CIRCULAR_COUNT; i++) {
		size_t result = reader.readIntoBuffer(buffer, SOUND_BUFFER_SIZE * reader.getChannels());
		alBufferData(this->buffers[i].bufferId, reader.getType(), (void*)buffer, result, reader.getSampleRate());

		if(result != SOUND_BUFFER_SIZE * reader.getChannels()) {
			this->bufferCount = i + 1;
			this->filled = false;
			break;
		}
	}

	delete[] buffer;
}

void _play(sound::SoundThreadContext* context) {
	sound::Sound* sound = context->sound;
	sound::SoundSourceProperties properties = context->properties;
	
	int bufferIndex = -SOUND_BUFFER_COUNT;
	ALuint source;
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, properties.pitch);
	alSourcef(source, AL_GAIN, properties.gain);
	alSource3f(source, AL_POSITION, properties.position.x, properties.position.y, properties.position.z);
	alSource3f(source, AL_VELOCITY, properties.velocity.x, properties.velocity.y, properties.velocity.z);
	alSourcei(source, AL_LOOPING, AL_FALSE);

	if(!sound->filled) {
		ALuint buffers[sound->bufferCount];
		for(unsigned int i = 0; i < sound->bufferCount; i++) {
			buffers[i] = sound->buffers[i].bufferId; // load up pre-loaded buffers
		}
		
		alSourceQueueBuffers(source, sound->bufferCount, buffers);
		alSourcePlay(source);

		ALint state = AL_PLAYING;
		while(state == AL_PLAYING) {
			alGetSourcei(source, AL_SOURCE_STATE, &state); // update the state
			this_thread::sleep_for(chrono::milliseconds(SOUND_THREAD_WAIT));
		}

		alDeleteSources(1, &source); // cleanup the source

		engine->soundEngine.finishThread(context);
		return;
	}

	sound::SoundFileType type = sound::OGG_FILE;
	if(sound->fileName.find("wav") != string::npos) {
		type = sound::WAV_FILE;
	}

	sound::SoundReader reader(sound->position, sound->size, type);

	// read data into the buffers
	char* buffer = new char[SOUND_BUFFER_SIZE * reader.getChannels()];

	if(type == sound::OGG_FILE) {
		reader.seek(SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT / 2.0); // seek some buffers worth into the .ogg stream
	}
	else if(type == sound::WAV_FILE) {
		reader.seek(SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT * reader.getChannels());
	}

	ALuint buffers[SOUND_BUFFER_CIRCULAR_COUNT];
	for(unsigned int i = 0; i < SOUND_BUFFER_CIRCULAR_COUNT; i++) {
		buffers[i] = sound->buffers[i].bufferId; // load up pre-loaded buffers
	}

	sound::Buffer* newBuffers = new sound::Buffer[SOUND_BUFFER_CIRCULAR_COUNT];

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
			poppedBuffer = newBuffers[bufferIndex + SOUND_BUFFER_COUNT].bufferId;
			bufferIndex++;
		}
		
		size_t result = reader.readIntoBuffer(buffer, SOUND_BUFFER_SIZE * reader.getChannels());
		if(result < 0) { // error
			break;
		}
		else if(result != SOUND_BUFFER_SIZE * reader.getChannels()) {
			done = true;
		}

		alBufferData(poppedBuffer, reader.getType(), (void*)buffer, result, reader.getSampleRate());
		alSourceQueueBuffers(source, 1, &poppedBuffer);
		buffersProcessed--;
	}

	alDeleteSources(1, &source);
	delete[] buffer;

	engine->soundEngine.finishThread(context);
}

void sound::Sound::play(SoundSourceProperties properties) {
	#ifdef __switch__
	Thread* thread = new Thread;
	SoundThreadContext* context = new SoundThreadContext;
	context->sound = this;
	context->properties = properties;
	context->thread = thread;
	Result result = threadCreate(thread, (void (*)(void*))::_play, context, NULL, 0x10000, 0x2C, 1);

	if(!R_SUCCEEDED(result)) {
		printf("failed to create sound thread\n");
		return;
	}
	
	result = threadStart(thread);

	if(!R_SUCCEEDED(result)) {
		printf("failed to start sound thread\n");
		return;
	}
	#else
	SoundThreadContext* context = new SoundThreadContext;
	context->properties = properties;
	context->sound = this;
	thread t(&::_play, context);
	t.detach();
	#endif
}
