#include "shaderSource.h"

#include <string.h>

resources::ShaderSource::ShaderSource(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize,
	bool useCharBuffer
) : ResourceObject(manager, metadata) {
	if(useCharBuffer) {
		this->buffer = new unsigned char[bufferSize];
		this->bufferSize = bufferSize;
		memcpy(this->buffer, buffer, bufferSize);
	}
	else {
		this->source = string((char*)buffer, bufferSize);
	}
}

unsigned int resources::ShaderSource::getBytesUsed() {
	if(this->bufferSize == 0) {
		return this->source.length();
	}
	else {
		return this->bufferSize;
	}
}
