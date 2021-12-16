#include "soundReader.h"

#include "../engine/engine.h"

sound::SoundReader::SoundReader(streampos location, size_t size, SoundFileType type) {
	this->location = location;
	this->size = size;
	this->type = type;
	this->file = ifstream(engine->getFilePrefix() + "out.carton");
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
			return this->wavFormat.sampleRate;
		}
	}
}

unsigned short sound::SoundReader::getBitDepth() {
	switch(this->type) {
		case OGG_FILE: {
			return 16;
		}

		case WAV_FILE: {
			return this->wavFormat.bitdepth;
		}
	}
}

size_t sound::SoundReader::readIntoBuffer(char* buffer, size_t bufferSize) {
	switch(this->type) {
		case OGG_FILE: {
			unsigned long pointer = 0;
			while(pointer < bufferSize) {
				long result = ov_read(this->ogg.file, &buffer[pointer], bufferSize - pointer, 0, 2, 1, &this->ogg.currentSection);
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
			break;
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
			break;
		}
	}
}
