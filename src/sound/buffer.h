#pragma once

#include <AL/al.h>

namespace sound {
	enum BufferFormat {
		MONO_8_BIT = AL_FORMAT_MONO8,
		MONO_16_BIT = AL_FORMAT_MONO16,
		STEREO_8_BIT = AL_FORMAT_STEREO8,
		STEREO_16_BIT = AL_FORMAT_STEREO16,
	};
	
	class Buffer {
		public:
			Buffer();
			~Buffer();
			
			ALuint bufferId;

			void setData(void* data, unsigned int size, unsigned int frequency, BufferFormat format);

			bool shared() {
				return this->_shared;
			}

		private:
			BufferFormat format;
			unsigned int size = 0;
			unsigned int frequency = 0;
			bool _shared = false;
	};
};
