#include "neighbors.h"

#include "chunkContainer.h"

TileNeighborIterator::TileNeighborIterator(ChunkContainer* container, glm::ivec3 position) {
	this->container = container; 
	this->position = position;
	this->iterateTile();
}

bool TileNeighborIterator::atEnd() {
	return this->index == -1;
}

bool TileNeighborIterator::operator==(const TileNeighborIterator &other) {
	return this->foundPosition == other.foundPosition;
}

TileNeighborIterator& TileNeighborIterator::operator++() {
	this->iterateTile();
	return *this;
}

glm::ivec3& TileNeighborIterator::value() {
	return this->foundPosition;
}

void TileNeighborIterator::iterateTile() {
	glm::ivec3 offsets[4] = {
		glm::ivec3(1, 0, 0),
		glm::ivec3(-1, 0, 0),
		glm::ivec3(0, 1, 0),
		glm::ivec3(0, -1, 0),
	};

	for(; this->index < 4; this->index++) {
		if(this->testTile(this->position + offsets[this->index])) {
			this->foundPosition = this->position + offsets[this->index];
			this->index++;
			return;
		}
	}	

	this->index = -1;
}

bool TileNeighborIterator::testTile(glm::ivec3 position) {
	if(this->container->getTile(position + glm::ivec3(0, 0, 1)) != 0 || this->container->positionToCharacter.find(position + glm::ivec3(0, 0, 1)) != this->container->positionToCharacter.end()) {
		return false;
	}
	return true;
}
