#include "css.h"

resources::CSS::CSS(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->styles = std::string((const char*)buffer, bufferSize);
}

void resources::CSS::reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
	ResourceObject::reload(metadata, buffer, bufferSize);
	this->styles = std::string((const char*)buffer, bufferSize);
}

unsigned int resources::CSS::getBytesUsed() {
	return this->styles.length();
}
