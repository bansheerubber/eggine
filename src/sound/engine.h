#pragma once

#ifdef __switch__
#include <switch.h>
#endif

#include <deque>
#include <eggscript/egg.h>
#include <mutex>
#include <tsl/robin_map.h>
#include <glm/vec3.hpp>
#include <vector>

using namespace std;

namespace es {
	void defineSoundEngine();
	esEntryPtr playSound(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
};

class ALCdevice;
class ALCcontext;
namespace sound {
	class Engine {
		friend class SoundCollection;
		
		public:
			Engine();

			void initialize();
			void tick();
			void setPosition(glm::vec3 position);
			void addSound(class Sound* file);
			void addCollection(class SoundCollection* collection);
			void playSoundByFileName(string fileName);
			void playSoundByCollectionName(string collectionName);

			void finishThread(struct SoundThreadContext* context);
		
		private:
			ALCdevice* device;
			ALCcontext* context;

			vector<class Sound*> sounds;
			tsl::robin_map<string, class Sound*> fileToSound;

			vector<class SoundCollection*> soundCollections;
			tsl::robin_map<string, class SoundCollection*> nameToCollection;

			deque<struct SoundThreadContext*> finishedThreads;
	};
}
