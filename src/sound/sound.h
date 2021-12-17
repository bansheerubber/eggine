#pragma once

#ifdef __switch__
#include <switch.h>
#endif

#include <vorbis/codec.h>
#include <fstream>
#include <vorbis/vorbisfile.h>

#include "buffer.h"
#include "../resources/resourceObject.h"

using namespace std;

namespace resources {
	class ResourceManager;
};

namespace sound {
	class Sound;

	struct SoundThreadContext {
		sound::Sound* sound;
		#ifdef __switch__
		Thread* thread;
		#endif
	};
};

void _play(sound::SoundThreadContext* context);

#define SOUND_BUFFER_SIZE 32768
#define SOUND_THREAD_WAIT 250
#define SOUND_BUFFER_CIRCULAR_COUNT 2
#define SOUND_BUFFER_COUNT 2 // amount of buffers we always store in memory, starting from the beginning of the file
namespace sound {
	class Sound: public resources::ResourceObject {
		friend class Engine;
		friend void ::_play(sound::SoundThreadContext* context);
		
		public:
			Sound(resources::ResourceManager* manager, carton::Metadata* metadata);

			void play();
			unsigned int getBytesUsed() {
				return SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT;
			}

		private:
			string fileName;
			streampos position; // position within the carton
			size_t size; // full size of the file
			Buffer buffers[SOUND_BUFFER_COUNT]; // initial buffers that we always keep in memory
			unsigned int bufferCount = 0;
			bool filled = true; // whether both buffers are filled all the way
	};
};
