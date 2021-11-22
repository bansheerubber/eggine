#include "html.h"

resources::HTML::HTML(
	ResourceManager* manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	size_t bufferSize
) : ResourceObject(manager, metadata) {
	this->document = string((const char*)buffer, bufferSize);
}
