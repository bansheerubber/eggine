#include "css.h"

resources::CSS::CSS(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->styles = string((const char*)buffer, bufferSize);
}
