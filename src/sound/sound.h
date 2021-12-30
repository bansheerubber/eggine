#pragma once

#ifdef __switch__
#include <switch.h>
#endif

#include <vorbis/codec.h>
#include <fstream>
#include <glm/vec3.hpp>
#include <vorbis/vorbisfile.h>

#include "buffer.h"
#include "../resources/resourceObject.h"

namespace resources {
	class ResourceManager;
};

namespace sound {
	class Sound;

	struct SoundSourceProperties {
		double pitch = 1.0;
		double gain = 1.0;
		glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
		glm::vec3 velocity = glm::vec3(0.0, 0.0, 0.0);
	};

	struct SoundThreadContext {
		sound::Sound* sound;
		SoundSourceProperties properties;
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

			void play(SoundSourceProperties properties = SoundSourceProperties());

			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				resources::ResourceObject::reload(metadata, buffer, bufferSize);
				printf("sound reload not implemented\n");
			}

			unsigned int getBytesUsed() {
				return SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT;
			}

		private:
			std::string fileName;
			uint64_t position; // position within the carton
			uint64_t size; // full size of the file
			Buffer buffers[SOUND_BUFFER_COUNT]; // initial buffers that we always keep in memory
			unsigned int bufferCount = 0;
			bool filled = true; // whether both buffers are filled all the way
	};
};
