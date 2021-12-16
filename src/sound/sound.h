#pragma once

#include <vorbis/codec.h>
#include <fstream>
#include <vorbis/vorbisfile.h>

#include "buffer.h"

using namespace std;

#define SOUND_BUFFER_COUNT 4 // amount of buffers we always store in memory, starting from the beginning of the file
namespace sound {
	class SoundFile {
		public:
			SoundFile(string fileName, streampos position, size_t size);
			~SoundFile();

			void play();
		
		private:
			string fileName;
			streampos position; // position within the carton
			size_t size; // full size of the file
			Buffer buffers[SOUND_BUFFER_COUNT]; // initial buffers that we always keep in memory
	};

	class SoundFileInstance {
		friend size_t ifstream_read(void* data, size_t size, size_t count, void* file);
		friend int ifstream_seek(void* file, ogg_int64_t offset, int origin);
		friend int ifstream_close(void* file);
		friend long ifstream_tell(void* file);
		
		public:
			SoundFileInstance(string fileName, streampos position, size_t size);

		private:
			ifstream file;
			string fileName;
			streampos position;
			size_t size;
	};

	inline size_t ifstream_read(void* data, size_t size, size_t count, void* file) {
		SoundFileInstance* soundFile = (SoundFileInstance*)file;

		size_t bytes = size * count;
		size_t end = soundFile->position + soundFile->size;
		size_t current = soundFile->file.tellg();
		size_t readAmount = min(end - current, bytes);

		soundFile->file.read((char*)data, readAmount);

		for(size_t i = 0; i < size * count; i++) {
			// printf("%c", ((char*)data)[i]);
		}
		// printf("\n");

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

	inline int ifstream_close(void* file) {
		((SoundFileInstance*)file)->file.close();
	}

	inline long ifstream_tell(void* file) {
		SoundFileInstance* soundFile = (SoundFileInstance*)file;
		return (size_t)soundFile->file.tellg() - soundFile->position;
	}
	
	static ov_callbacks OV_CALLBACKS_IFSTREAM = {
		(size_t (*)(void *, size_t, size_t, void *))  ifstream_read,
		(int (*)(void *, ogg_int64_t, int))           ifstream_seek,
		(int (*)(void *))                             ifstream_close,
		(long (*)(void *))                            ifstream_tell
	};
};
