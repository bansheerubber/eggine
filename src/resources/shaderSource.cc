#include "shaderSource.h"

#include <string.h>

resources::ShaderSource::ShaderSource(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->buffer = new unsigned char[bufferSize];
	this->bufferSize = bufferSize;
	memcpy(this->buffer, buffer, bufferSize);

	this->fileName = metadata->getMetadata("fileName");
}

unsigned int resources::ShaderSource::getBytesUsed() {
	return this->bufferSize;
}
