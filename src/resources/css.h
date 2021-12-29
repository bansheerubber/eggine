#pragma once

#include "resourceObject.h"

namespace resources {
	class CSS: public ResourceObject {
		public:
			CSS(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			string styles;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			unsigned int getBytesUsed();
	};
};
