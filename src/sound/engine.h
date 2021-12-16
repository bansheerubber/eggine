#pragma once

#include <tsl/robin_map.h>
#include <glm/vec3.hpp>
#include <vector>

using namespace std;

class ALCdevice;
class ALCcontext;
namespace sound {
	class Engine {
		public:
			Engine();

			void initialize();
			void setPosition(glm::vec3 position);
			void addSound(class Sound* file);
			void playSoundByFileName(string fileName);
		
		private:
			ALCdevice* device;
			ALCcontext* context;

			vector<class Sound*> sounds;
			tsl::robin_map<string, class Sound*> fileToSound;
	};
}
