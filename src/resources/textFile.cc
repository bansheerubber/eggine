#include "textFile.h"

resources::TextFile::TextFile(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->source = std::string((const char*)buffer, bufferSize);
}

void resources::TextFile::reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
	ResourceObject::reload(metadata, buffer, bufferSize);
	this->source = std::string((const char*)buffer, bufferSize);
}

unsigned int resources::TextFile::getBytesUsed() {
	return this->source.length();
}

std::string resources::TextFile::getSource() {
	return this->source;
}
