#pragma once

#include "../resources/resourceObject.h"

namespace resources {
	class ResourceManager;
};

namespace sound {
	class SoundCollection : public resources::ResourceObject	 {
		friend class Engine;
		
		public:
			SoundCollection(resources::ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);

			void play(); // pick a random sound from the collection and play it

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				resources::ResourceObject::reload(metadata, buffer, bufferSize);
				printf("sound collection reload not implemented\n");
			}
			
			unsigned int getBytesUsed() {
				return 0;
			}
		
		private:
			string name = "";
			vector<class Sound*> sounds;

			double pitchMin = 1.0;
			double pitchMax = 1.0;
	};
};
