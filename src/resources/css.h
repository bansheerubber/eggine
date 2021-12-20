#pragma once

#include "resourceObject.h"

namespace resources {
	class CSS: public ResourceObject {
		public:
			CSS(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);
			string styles;

			void reload(carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);
			unsigned int getBytesUsed();
	};
};
