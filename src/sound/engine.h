#pragma once

#ifdef __switch__
#include <switch.h>
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <deque>
#include <eggscript/egg.h>
#include <mutex>
#include <tsl/robin_map.h>
#include <glm/vec3.hpp>
#include <vector>

namespace es {
	void defineSoundEngine();
	esEntryPtr playSound(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
};

namespace sound {
	class Engine {
		friend class SoundCollection;
		
		public:
			Engine();
			~Engine();

			void initialize();
			void tick();
			void setPosition(glm::vec3 position);
			void addSound(class Sound* file);
			void addCollection(class SoundCollection* collection);
			void playSoundByFileName(std::string fileName);
			void playSoundByCollectionName(std::string collectionName);

			void finishThread(struct SoundThreadContext* context);
		
		private:
			ALCdevice* device;
			ALCcontext* context;

			std::vector<class Sound*> sounds;
			tsl::robin_map<std::string, class Sound*> fileToSound;

			std::vector<class SoundCollection*> soundCollections;
			tsl::robin_map<std::string, class SoundCollection*> nameToCollection;

			std::deque<struct SoundThreadContext*> finishedThreads;
	};
}
