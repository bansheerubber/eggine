#pragma once

#include "../engine/console.h"
#include "resourceObject.h"

class Map;

namespace resources {
	class MapSource: public ResourceObject {
		public:
			MapSource(ResourceManager &manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);
			~MapSource();

			void loadIntoMap(Map* map);
			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				ResourceObject::reload(metadata, buffer, bufferSize);
				console::warning("map source reload not implemented\n");
			}
			unsigned int getBytesUsed();
		
		private:
			unsigned char* buffer;
			uint64_t bufferSize;
	};
};
