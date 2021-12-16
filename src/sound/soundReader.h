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

			struct {
				OggVorbis_File* file;
				vorbis_info* info;
				int currentSection = 0;
			} ogg;

			struct {
				int32_t subchunkId;
				int32_t subchunkSize;
				int16_t audioFormat;
				int16_t channels;
				int32_t sampleRate;
				int32_t byteRate;
				int16_t blockAlign;
				int16_t bitdepth;
			} wavFormat;
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
