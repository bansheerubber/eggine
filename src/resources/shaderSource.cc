#include "shaderSource.h"

resources::ShaderSource::ShaderSource(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->source = string((char*)buffer, bufferSize);
}
