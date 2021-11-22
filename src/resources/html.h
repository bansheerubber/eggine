#pragma once

#include "resourceObject.h"

namespace resources {
	class HTML: public ResourceObject {
		public:
			HTML(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);
			string document;
	};
};
