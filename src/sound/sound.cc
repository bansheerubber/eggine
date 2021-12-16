#include "sound.h"

#include <iostream>
#include <thread>

#include "../engine/engine.h"

sound::SoundFile::SoundFile(
	resources::ResourceManager* manager,
	carton::Metadata* metadata,
	string fileName,
	streampos position,
	size_t size
) : resources::ResourceObject(manager, metadata) {
	this->fileName = fileName;
	this->position = position;
	this->size = size;

	SoundFileInstance instance(this, this->fileName, this->position, this->size);
	OggVorbis_File file;
	if(ov_open_callbacks(&instance, &file, NULL, 0, OV_CALLBACKS_IFSTREAM) < 0) {
		printf("Could not read sound '%s'\n", this->fileName.c_str());
		exit(1);
	}

	vorbis_info* info = ov_info(&file, -1);

	// read data into the buffers
	char* buffer = new char[SOUND_BUFFER_SIZE];

	int currentSection = 0;
	for(size_t i = 0; i < SOUND_BUFFER_CIRCULAR_COUNT; i++) {
		int result = SoundFile::ReadIntoBuffer(&file, buffer, SOUND_BUFFER_SIZE, &currentSection);
		this->buffers[i].setData(buffer, result, info->rate, MONO_16_BIT);

		if(result != SOUND_BUFFER_SIZE) {
			this->bufferCount = i + 1;
			this->filled = false;
			break;
		}
	}

	ov_clear(&file);
	delete[] buffer;
}

sound::SoundFile::~SoundFile() {
	
}

void sound::SoundFile::play() {
	SoundFileInstance* instance = new SoundFileInstance(this, this->fileName, this->position, this->size);
	thread t(&SoundFileInstance::play, instance);
	t.detach();
}

void sound::SoundFileInstance::play() {
	if(!this->parent->filled) {
		ALuint buffers[this->parent->bufferCount];
		for(unsigned int i = 0; i < this->parent->bufferCount; i++) {
			buffers[i] = this->parent->buffers[i].bufferId; // load up pre-loaded buffers
		}
		
		alSourceQueueBuffers(this->source, this->parent->bufferCount, buffers);
		alSourcePlay(this->source);

		ALint state = AL_PLAYING;
		while(state == AL_PLAYING) {
			alGetSourcei(this->source, AL_SOURCE_STATE, &state); // update the state
			this_thread::sleep_for(chrono::milliseconds(SOUND_THREAD_WAIT));
		}

		delete this;
		return;
	}

	OggVorbis_File file;
	if(ov_open_callbacks(this, &file, NULL, 0, OV_CALLBACKS_IFSTREAM) < 0) {
		printf("Could not read sound '%s'\n", this->fileName.c_str());
		exit(1);
	}

	vorbis_info* info = ov_info(&file, -1);

	// read data into the buffers
	char* buffer = new char[SOUND_BUFFER_SIZE];

	ov_pcm_seek(&file, SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT / 2.0); // seek some buffers worth into the .ogg stream

	int currentSection = 0;

	ALuint buffers[SOUND_BUFFER_CIRCULAR_COUNT];
	for(unsigned int i = 0; i < SOUND_BUFFER_CIRCULAR_COUNT; i++) {
		buffers[i] = this->parent->buffers[i].bufferId; // load up pre-loaded buffers
	}

	alSourceQueueBuffers(this->source, SOUND_BUFFER_CIRCULAR_COUNT, buffers);
	alSourcePlay(this->source);

	ALint state = AL_PLAYING;
	bool done = false;
	while(state == AL_PLAYING) {
		alGetSourcei(this->source, AL_SOURCE_STATE, &state); // update the state
		
		ALint buffersProcessed = 0;
		alGetSourcei(this->source, AL_BUFFERS_PROCESSED, &buffersProcessed); // figure out if we need to update buffers

		if(buffersProcessed == 0) {
			this_thread::sleep_for(chrono::milliseconds(SOUND_THREAD_WAIT));
			continue;
		}

		// go through the queue and update buffers
		ALuint poppedBuffer;
		if(done) {
			alSourceUnqueueBuffers(this->source, 1, &poppedBuffer);
			break;
		}
		
		alSourceUnqueueBuffers(this->source, 1, &poppedBuffer);

		if(this->bufferIndex < 0) {
			poppedBuffer = this->buffers[this->bufferIndex + SOUND_BUFFER_COUNT].bufferId;
			this->bufferIndex++;
		}
		
		int result = SoundFile::ReadIntoBuffer(&file, buffer, SOUND_BUFFER_SIZE, &currentSection);
		if(result < 0) { // error
			ov_clear(&file);
			delete[] buffer;
			delete this;
			return;
		}
		else if(result != SOUND_BUFFER_SIZE) {
			done = true;
		}

		alBufferData(poppedBuffer, AL_FORMAT_MONO16, (void*)buffer, result, info->rate);
		alSourceQueueBuffers(this->source, 1, &poppedBuffer);
		buffersProcessed--;
	}

	ov_clear(&file);
	delete[] buffer;
	delete this;
}

int sound::SoundFile::ReadIntoBuffer(OggVorbis_File* file, char* buffer, size_t bufferSize, int* currentSection) {
	unsigned long pointer = 0;
	while(pointer < bufferSize) {
		long result = ov_read(file, &buffer[pointer], bufferSize - pointer, 0, 2, 1, currentSection);
		if(result == OV_HOLE) {
			return -1;
		}
		else if(result == OV_EBADLINK) {
			return -1;
		}
		else if(result == OV_EINVAL) {
			return -1;
		}
		else if(result == 0) {
			return pointer;
		}

		pointer += result;
	}
	return pointer;
}

sound::SoundFileInstance::SoundFileInstance(SoundFile* parent, string fileName, streampos position, size_t size) {
	this->parent = parent;
	this->fileName = fileName;
	this->position = position;
	this->size = size;

	this->file = ifstream(engine->getFilePrefix() + "out.carton");
	this->file.seekg(this->position);

	alGenSources(1, &this->source);
	alSourcef(this->source, AL_PITCH, 1);
	alSourcef(this->source, AL_GAIN, 1.0f);
	alSource3f(this->source, AL_POSITION, 0, 0, 0);
	alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
	alSourcei(this->source, AL_LOOPING, AL_FALSE);
}

sound::SoundFileInstance::~SoundFileInstance() {
	alDeleteSources(1, &this->source);
}
