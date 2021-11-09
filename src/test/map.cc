#include "map.h"

#include "chunkContainer.h"
#include "tileMath.h"

Map::Map(ChunkContainer* container) {
	this->container = container;
}

void Map::loadFromFile(string filename) {
	ifstream file(filename);

	if(file.bad() || file.fail()) {
		printf("failed to open file for map %s\n", filename.c_str());
		file.close();
		return;
  }

	file.seekg(0, file.end);
	unsigned long length = file.tellg();
	file.seekg(0, file.beg);
	char* buffer = new char[length];
	file.read((char*)buffer, length);
	file.close();

	this->load((unsigned char*)buffer, length);
	delete[] buffer;
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
	
	uint16_t chunkSize;
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
				uint16_t height = this->readNumber<uint16_t>(&buffer[index], &index);

				Chunk &chunk = this->container->addChunk(glm::uvec2(x, y));
				for(unsigned int i = 0; i < (unsigned int)chunkSize * (unsigned int)chunkSize * (unsigned int)height; i++) {
					uint16_t tile = this->readNumber<uint16_t>(&buffer[index], &index);
					chunk.setTileTextureByIndex(i, tile);
				}
				break;
			}

			case MAP_SIZE: {
				uint16_t size = this->readNumber<uint16_t>(&buffer[index], &index);
				this->container->setSize(size);
				chunkSize = this->readNumber<uint16_t>(&buffer[index], &index);
				break;
			}

			default: {
				printf("error reading map\n");
				exit(0);
			}
		}
	}
}

void Map::save(string filename) {
	ofstream file(filename);

	file.write("XMAP", 4);

	this->writeNumber<uint64_t>(file, Version);

	// map size command
	this->writeNumber<short>(file, MAP_SIZE);
	this->writeNumber<short>(file, this->container->getSize());
	this->writeNumber<short>(file, Chunk::Size);

	for(unsigned int i = 0; i < this->container->getChunkCount(); i++) {
		Chunk &chunk = this->container->getChunk(i);

		// map chunk command
		this->writeNumber<short>(file, MAP_CHUNK);
		this->writeNumber<short>(file, chunk.getPosition().x);
		this->writeNumber<short>(file, chunk.getPosition().y);
		this->writeNumber<short>(file, chunk.height);
		for(unsigned int i = 0; i < Chunk::Size * Chunk::Size * chunk.height; i++) {
			this->writeNumber<short>(file, chunk.getTileTextureByIndex(i));
		}
	}

	file.close();
}
