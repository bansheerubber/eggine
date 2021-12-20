#include "css.h"

resources::CSS::CSS(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->styles = string((const char*)buffer, bufferSize);
}

void resources::CSS::reload(carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize) {
	ResourceObject::reload(metadata, buffer, bufferSize);
	this->styles = string((const char*)buffer, bufferSize);
}

unsigned int resources::CSS::getBytesUsed() {
	return this->styles.length();
}
