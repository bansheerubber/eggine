#include "map.h"

Map::Map(ChunkContainer* container) {
	this->container = container;
}

void Map::load(unsigned char* buffer, unsigned long size) {
	const char header[4] = {'X', 'M', 'A', 'P'};
	unsigned long index = 0;
	for(; index < sizeof(header); index++) {
		if(buffer[index] != header[index]) {
			printf("invalid map header\n");
			exit(0);
		}
	}

	uint64_t version = this->readNumber<uint64_t>(&buffer[index], &index);
	if(version != Version) {
		printf("invalid map version %#016lx\n", version);
		exit(1);
	}
	
	while((index + sizeof(uint16_t)) < size) {
		MapCommand command = (MapCommand)this->readNumber<uint16_t>(&buffer[index], &index);
		switch(command) {
			case MAP_INFO: {
				string key = this->readString(&buffer[index], &index);
				string value = this->readString(&buffer[index], &index);
				this->info[key] = value;
				break;
			}

			case MAP_CHUNK: {
				uint16_t x = this->readNumber<uint16_t>(&buffer[index], &index);
				uint16_t y = this->readNumber<uint16_t>(&buffer[index], &index);
				uint16_t size = this->readNumber<uint16_t>(&buffer[index], &index);
				uint16_t height = this->readNumber<uint16_t>(&buffer[index], &index);
				for(unsigned int i = 0; i < size * size * height; i++) {
					uint16_t tile = this->readNumber<uint16_t>(&buffer[index], &index);
				}
				break;
			}

			case MAP_SIZE: {
				uint16_t size = this->readNumber<uint16_t>(&buffer[index], &index);
				break;
			}

			default: {
				printf("error reading map\n");
				exit(0);
			}
		}
	}
}