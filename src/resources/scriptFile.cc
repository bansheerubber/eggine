#include "scriptFile.h"

resources::ScriptFile::ScriptFile(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->script = std::string((const char*)buffer, bufferSize);
}

void resources::ScriptFile::reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
	ResourceObject::reload(metadata, buffer, bufferSize);
	this->script = std::string((const char*)buffer, bufferSize);
}

unsigned int resources::ScriptFile::getBytesUsed() {
	return this->script.length();
}
