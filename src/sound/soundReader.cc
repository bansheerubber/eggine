#include "soundReader.h"

#include "../engine/engine.h"

static ov_callbacks OV_CALLBACKS_IFSTREAM = {
	(size_t (*)(void *, size_t, size_t, void *))  sound::ifstream_read,
	(int (*)(void *, ogg_int64_t, int))           sound::ifstream_seek,
	(int (*)(void *))                             nullptr,
	(long (*)(void *))                            sound::ifstream_tell
};

sound::SoundReader::SoundReader(uint64_t location, size_t size, SoundFileType type) {
	this->location = location;
	this->size = size;
	this->type = type;
	this->file = ifstream(engine->getFilePrefix() + "out.carton", ios::binary);
	this->file.seekg(location);

	switch(this->type) {
		case OGG_FILE: {
			this->ogg.file = new OggVorbis_File;
			if(ov_open_callbacks(this, this->ogg.file, NULL, 0, OV_CALLBACKS_IFSTREAM) < 0) {
				printf("Could not read .ogg sound\n");
				exit(1);
			}

			this->ogg.info = ov_info(this->ogg.file, -1);
			break;
		}

		case WAV_FILE: {
			uint32_t riffId = this->readNumber<uint32_t>();
			if(riffId != 0x46464952) {
				printf("Could not read .wav sound\n");
				exit(1);
			}

			this->readNumber<uint32_t>(); // file size

			uint32_t waveMagicNumber = this->readNumber<uint32_t>();
			if(waveMagicNumber != 0x45564157) {
				printf("Could not read .wav magic number\n");
				exit(1);
			}

			this->file.read((char*)&(this->wav.format), sizeof(this->wav.format));

			uint32_t section = this->readNumber<uint32_t>();
			uint32_t size = this->readNumber<uint32_t>();
			while(section != 0x61746164) {
				this->file.seekg(size, ios_base::cur);
				section = this->readNumber<uint32_t>();
				size = this->readNumber<uint32_t>();
			}
			
			if(!this->file.eof()) {
				this->wav.dataLocation = (uint64_t)this->file.tellg();
				this->wav.dataSize = size;
			}
			else {
				printf("Could not find .wav data section\n");
				exit(1);
			}
			
			break;
		}
	}
}

sound::SoundReader::~SoundReader() {
	switch(this->type) {
		case OGG_FILE: {
			ov_clear(this->ogg.file);
			break;
		}

		case WAV_FILE: {
			break;
		}
	}
}

unsigned int sound::SoundReader::getSampleRate() {
	switch(this->type) {
		case OGG_FILE: {
			return this->ogg.info->rate;
		}

		case WAV_FILE: {
			return this->wav.format.sampleRate;
		}

		default: {
			return 0;
		}
	}
}

unsigned short sound::SoundReader::getBitDepth() {
	switch(this->type) {
		case OGG_FILE: {
			return 16;
		}

		case WAV_FILE: {
			return this->wav.format.bitdepth;
		}

		default: {
			return 0;
		}
	}
}

unsigned short sound::SoundReader::getChannels() {
	switch(this->type) {
		case OGG_FILE: {
			return this->ogg.info->channels;
		}

		case WAV_FILE: {
			return this->wav.format.channels;
		}

		default: {
			return 0;
		}
	}
}

ALenum sound::SoundReader::getType() {
	if(this->getChannels() == 1) {
		if(this->getBitDepth() == 8) {
			return AL_FORMAT_MONO8;
		}
		else {
			return AL_FORMAT_MONO16;
		}
	}
	else {
		if(this->getBitDepth() == 8) {
			return AL_FORMAT_STEREO8;
		}
		else {
			return AL_FORMAT_STEREO16;
		}
	}
}

size_t sound::SoundReader::readIntoBuffer(char* buffer, size_t bufferSize) {
	switch(this->type) {
		case OGG_FILE: {
			uint64_t pointer = 0;
			while(pointer < bufferSize) {
				int64_t result = ov_read(this->ogg.file, &buffer[pointer], bufferSize - pointer, 0, 2, 1, &this->ogg.currentSection);
				if(result == OV_HOLE) {
					return -1;
				}
				else if(result == OV_EBADLINK) {
					return -1;
				}
				else if(result == OV_EINVAL) {
					return -1;
				}
				else if(result == 0) {
					return pointer;
				}

				pointer += result;
			}
			return pointer;
		}

		case WAV_FILE: {
			size_t bytesLeft = this->wav.dataSize - ((uint64_t)this->file.tellg() - this->wav.dataLocation);
			size_t readSize = min(bytesLeft, bufferSize);
			this->file.read(buffer, readSize);
			return readSize;
		}

		default: {
			return 0;
		}
	}
}

void sound::SoundReader::seek(size_t location) {
	switch(this->type) {
		case OGG_FILE: {
			ov_pcm_seek(this->ogg.file, location);
			break;
		}

		case WAV_FILE: {
			this->file.seekg((size_t)this->wav.dataLocation + location);
			break;
		}
	}
}
