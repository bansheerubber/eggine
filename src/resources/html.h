#pragma once

#include "resourceObject.h"

namespace resources {
	class HTML: public ResourceObject {
		public:
			HTML(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			string document;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			unsigned int getBytesUsed();
	};
};
