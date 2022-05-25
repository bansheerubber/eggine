#include "html.h"

resources::HTML::HTML(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->document = std::string((const char*)buffer, bufferSize);
}

void resources::HTML::reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
	ResourceObject::reload(metadata, buffer, bufferSize);
	this->document = std::string((const char*)buffer, bufferSize);
}

unsigned int resources::HTML::getBytesUsed() {
	return this->document.length();
}
