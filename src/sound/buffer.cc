#include "buffer.h"

sound::Buffer::Buffer() {
	alGenBuffers(1, &this->bufferId);
}

sound::Buffer::~Buffer() {
	alDeleteBuffers(1, &this->bufferId);
}

void sound::Buffer::setData(void* data, unsigned int size, unsigned int frequency, BufferFormat format) {
	this->size = size;
	this->frequency = frequency;
	this->format = format;
	alBufferData(this->bufferId, (ALenum)format, data, size, frequency);
}
