#pragma once

#include <vorbis/codec.h>
#include <fstream>
#include <vorbis/vorbisfile.h>

#include "buffer.h"
#include "../resources/resourceObject.h"

using namespace std;

namespace resources {
	class ResourceManager;
};

#define SOUND_BUFFER_SIZE 32768
#define SOUND_THREAD_WAIT 250
#define SOUND_BUFFER_CIRCULAR_COUNT 2
#define SOUND_BUFFER_COUNT 2 // amount of buffers we always store in memory, starting from the beginning of the file
namespace sound {
	class SoundFile: public resources::ResourceObject {
		friend class SoundFileInstance;
		friend class Engine;
		
		public:
			SoundFile(
				resources::ResourceManager* manager,
				carton::Metadata* metadata,
				string fileName,
				streampos position,
				size_t size
			);
			~SoundFile();

			void play();
			unsigned int getBytesUsed() {
				return SOUND_BUFFER_SIZE * SOUND_BUFFER_COUNT;
			}

			static int ReadIntoBuffer(OggVorbis_File* file, char* buffer, size_t bufferSize, int* currentSection);
		
		private:
			string fileName;
			streampos position; // position within the carton
			size_t size; // full size of the file
			Buffer buffers[SOUND_BUFFER_COUNT]; // initial buffers that we always keep in memory
			unsigned int bufferCount = 0;
			bool filled = true; // whether both buffers are filled all the way
	};

	class SoundFileInstance {
		friend SoundFile;
		friend size_t ifstream_read(void* data, size_t size, size_t count, void* file);
		friend int ifstream_seek(void* file, ogg_int64_t offset, int origin);
		friend int ifstream_close(void* file);
		friend long ifstream_tell(void* file);
		
		public:
			SoundFileInstance(SoundFile* parent, string fileName, streampos position, size_t size);
			~SoundFileInstance();

			void play();

		private:
			SoundFile* parent = nullptr;
			ifstream file;
			string fileName;
			streampos position;
			size_t size;

			Buffer buffers[SOUND_BUFFER_CIRCULAR_COUNT];
			int bufferIndex = -SOUND_BUFFER_COUNT; // negative indices are used to indicate shared buffers

			ALuint source;
	};

	inline size_t ifstream_read(void* data, size_t size, size_t count, void* file) {
		SoundFileInstance* soundFile = (SoundFileInstance*)file;

		size_t bytes = size * count;
		size_t end = soundFile->position + soundFile->size;
		size_t current = soundFile->file.tellg();
		size_t readAmount = min(end - current, bytes);

		soundFile->file.read((char*)data, readAmount);

		return soundFile->file.gcount() / size;
	}

	inline int ifstream_seek(void* file, ogg_int64_t offset, int origin) {
		int state = -1;
		SoundFileInstance* soundFile = (SoundFileInstance*)file;
		if(origin == SEEK_SET) {
			soundFile->file.seekg(offset + soundFile->position, ios_base::beg);
			state = soundFile->file.good() ? 0 : -1;
		}
		else if(origin == SEEK_CUR) {
			soundFile->file.seekg(offset, ios_base::cur);
			state = soundFile->file.good() ? 0 : -1;
		}
		else if(origin == SEEK_END) {
			soundFile->file.seekg(soundFile->position + soundFile->size - offset, ios_base::beg);
			state = soundFile->file.good() ? 0 : -1;
		}

		return state;
	}


	inline long ifstream_tell(void* file) {
		SoundFileInstance* soundFile = (SoundFileInstance*)file;
		return (size_t)soundFile->file.tellg() - soundFile->position;
	}
	
	static ov_callbacks OV_CALLBACKS_IFSTREAM = {
		(size_t (*)(void *, size_t, size_t, void *))  ifstream_read,
		(int (*)(void *, ogg_int64_t, int))           ifstream_seek,
		(int (*)(void *))                             nullptr,
		(long (*)(void *))                            ifstream_tell
	};
};
