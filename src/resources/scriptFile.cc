#include "scriptFile.h"

resources::ScriptFile::ScriptFile(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->script = string((const char*)buffer, bufferSize);
}