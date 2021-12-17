#pragma once

#include "../resources/resourceObject.h"

namespace resources {
	class ResourceManager;
};

namespace sound {
	class SoundCollection : public resources::ResourceObject	 {
		friend class Engine;
		
		public:
			SoundCollection(resources::ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);

			void play(); // pick a random sound from the collection and play it

			unsigned int getBytesUsed() {
				return 0;
			}
		
		private:
			string name = "";
			vector<class Sound*> sounds;
	};
};
