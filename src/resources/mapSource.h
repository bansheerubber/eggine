#pragma once

#include "resourceObject.h"

class Map;

namespace resources {
	class MapSource: public ResourceObject {
		public:
			MapSource(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);
			void loadIntoMap(Map* map);
			unsigned int getBytesUsed();
		
		private:
			unsigned char* buffer;
			size_t bufferSize;
	};
};
