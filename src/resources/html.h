#pragma once

#include "resourceObject.h"

namespace resources {
	class HTML: public ResourceObject {
		public:
			HTML(ResourceManager &manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			std::string document;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			unsigned int getBytesUsed();
	};
};
