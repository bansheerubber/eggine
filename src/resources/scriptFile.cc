#include "scriptFile.h"

resources::ScriptFile::ScriptFile(ResourceManager* manager, const unsigned char* buffer, size_t bufferSize) : ResourceObject(manager) {
	this->script = string((const char*)buffer, bufferSize);
}