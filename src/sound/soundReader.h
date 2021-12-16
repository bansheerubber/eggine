#pragma once

#include <vorbis/codec.h>
#include <fstream>
#include <vorbis/vorbisfile.h>

using namespace std;

namespace sound {
	enum SoundFileType {
		OGG_FILE,
		WAV_FILE,
	};
	
	class SoundReader {
		friend size_t ifstream_read(void* data, size_t size, size_t count, void* file);
		friend int ifstream_seek(void* file, ogg_int64_t offset, int origin);
		friend int ifstream_close(void* file);
		friend long ifstream_tell(void* file);
		
		public:
			SoundReader(streampos location, size_t size, SoundFileType type);
			~SoundReader();

			unsigned int getSampleRate();
			unsigned short getBitDepth();
			size_t readIntoBuffer(char* buffer, size_t bufferSize);
			void seek(size_t location);
		
		private:
			streampos location;
			size_t size;
			SoundFileType type;
			ifstream file;

			template<class T>
			T readNumber() {
				char bytes[sizeof(T)];
				this->file.read(bytes, sizeof(T));
				// char reversed[sizeof(T)];
				// for(int i = sizeof(T) - 1, j = 0; i >= 0; i--, j++) {
				// 	reversed[j] = bytes[i];
				// }
				return *((T*)(bytes));
			}

			struct {
				OggVorbis_File* file;
				vorbis_info* info;
				int currentSection = 0;
			} ogg;

			struct {
				struct {
					uint32_t subchunkId;
					uint32_t subchunkSize;
					uint16_t audioFormat;
					uint16_t channels;
					uint32_t sampleRate;
					uint32_t byteRate;
					uint16_t blockAlign;
					uint16_t bitdepth;
				} format;
				streampos dataLocation;
				uint32_t dataSize;
			} wav;
	};

	inline size_t ifstream_read(void* data, size_t size, size_t count, void* file) {
		SoundReader* soundReader = (SoundReader*)file;

		size_t bytes = size * count;
		size_t end = soundReader->location + soundReader->size;
		size_t current = soundReader->file.tellg();
		size_t readAmount = min(end - current, bytes);

		soundReader->file.read((char*)data, readAmount);

		return soundReader->file.gcount() / size;
	}

	inline int ifstream_seek(void* file, ogg_int64_t offset, int origin) {
		int state = -1;
		SoundReader* soundReader = (SoundReader*)file;
		if(origin == SEEK_SET) {
			soundReader->file.seekg(offset + soundReader->location, ios_base::beg);
			state = soundReader->file.good() ? 0 : -1;
		}
		else if(origin == SEEK_CUR) {
			soundReader->file.seekg(offset, ios_base::cur);
			state = soundReader->file.good() ? 0 : -1;
		}
		else if(origin == SEEK_END) {
			soundReader->file.seekg(soundReader->location + soundReader->size - offset, ios_base::beg);
			state = soundReader->file.good() ? 0 : -1;
		}

		return state;
	}


	inline long ifstream_tell(void* file) {
		SoundReader* soundReader = (SoundReader*)file;
		return (size_t)soundReader->file.tellg() - soundReader->location;
	}
	
	static ov_callbacks OV_CALLBACKS_IFSTREAM = {
		(size_t (*)(void *, size_t, size_t, void *))  ifstream_read,
		(int (*)(void *, ogg_int64_t, int))           ifstream_seek,
		(int (*)(void *))                             nullptr,
		(long (*)(void *))                            ifstream_tell
	};
};
