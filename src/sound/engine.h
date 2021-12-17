#pragma once

#ifdef __switch__
#include <switch.h>
#endif

#include <deque>
#include <mutex>
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
			void tick();
			void setPosition(glm::vec3 position);
			void addSound(class Sound* file);
			void playSoundByFileName(string fileName);

			void finishThread(struct SoundThreadContext* context);
		
		private:
			ALCdevice* device;
			ALCcontext* context;

			vector<class Sound*> sounds;
			tsl::robin_map<string, class Sound*> fileToSound;

			deque<struct SoundThreadContext*> finishedThreads;
	};
}
