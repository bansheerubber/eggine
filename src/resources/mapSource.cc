#include "mapSource.h"

#include "../test/map.h"
#include "resourceManager.h"

resources::MapSource::MapSource(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->buffer = new unsigned char[bufferSize];
	memcpy(this->buffer, buffer, bufferSize);
	this->bufferSize = bufferSize;
}

void resources::MapSource::loadIntoMap(Map* map) {
	map->load(this->buffer, this->bufferSize);
}