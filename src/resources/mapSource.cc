#include "mapSource.h"

#include "../test/map.h"
#include "resourceManager.h"

resources::MapSource::MapSource(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->buffer = new unsigned char[bufferSize];
	memcpy(this->buffer, buffer, bufferSize);
	this->bufferSize = bufferSize;
}

resources::MapSource::~MapSource() {
	delete[] this->buffer;
}

void resources::MapSource::loadIntoMap(Map* map) {
	map->load(this->buffer, this->bufferSize);
}

unsigned int resources::MapSource::getBytesUsed() {
	return this->bufferSize;
}
