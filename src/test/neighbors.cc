#include "neighbors.h"

#include "chunkContainer.h"
#include "../util/lsb.h"

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
		glm::ivec3(0, -1, 0), // north
		glm::ivec3(1, 0, 0), // east
		glm::ivec3(0, 1, 0), // south
		glm::ivec3(-1, 0, 0), // west
	};

	NeighborDirection directionsForWall[8][3] = {
		{DIRECTION_EAST, DIRECTION_SOUTH, DIRECTION_WEST}, // north-facing wall neighbors
		{DIRECTION_NORTH, DIRECTION_SOUTH, DIRECTION_WEST}, // east-facing wall neighbors
		{DIRECTION_NORTH, DIRECTION_EAST, DIRECTION_WEST}, // south-facing wall neighbors
		{DIRECTION_NORTH, DIRECTION_EAST, DIRECTION_SOUTH}, // west-facing wall neighbors
		{DIRECTION_SOUTH, DIRECTION_WEST, INVALID_DIRECTION}, // north-east corner wall neighbors
		{DIRECTION_NORTH, DIRECTION_WEST, INVALID_DIRECTION}, // south-east corner wall neighbors
		{DIRECTION_NORTH, DIRECTION_EAST, INVALID_DIRECTION}, // south-west corner wall neighbors
		{DIRECTION_EAST, DIRECTION_SOUTH, INVALID_DIRECTION}, // north-west corner wall neighbors
	};

	resources::SpriteSheetInfo info = this->container->getSpriteInfo(this->position + glm::ivec3(0, 0, 1));
	if(info.wall != resources::NO_WALL) {
		for(; this->index < 3; this->index++) {
			NeighborDirection direction = directionsForWall[lsb(info.wall)][this->index];
			if(direction == INVALID_DIRECTION) {
				continue;
			}

			glm::ivec3 offset = offsets[(unsigned int)direction - 1]; // look up the offset in the dictionary
			// printf("%u, %d %d %d\n", (unsigned int)direction, offset.x, offset.y, offset.z);
			
			if(this->testTile(this->position + offset, direction)) {
				this->foundPosition = this->position + offset;
				this->index++;
				return;
			}
		}
	}
	else {
		for(; this->index < 4; this->index++) {
			if(this->testTile(this->position + offsets[this->index], (NeighborDirection)(this->index + 1))) {
				this->foundPosition = this->position + offsets[this->index];
				this->index++;
				return;
			}
		}
	}

	this->index = -1;
}

bool TileNeighborIterator::testTile(glm::ivec3 position, NeighborDirection direction) {
	NeighborDirection invalidDirectionsForWall[8][2] = {
		{DIRECTION_SOUTH, INVALID_DIRECTION}, // north-facing wall
		{DIRECTION_WEST, INVALID_DIRECTION}, // east-facing wall
		{DIRECTION_NORTH, INVALID_DIRECTION}, // south-facing wall
		{DIRECTION_EAST, INVALID_DIRECTION}, // west-facing wall
		{DIRECTION_SOUTH, DIRECTION_WEST}, // north-east corner wall
		{DIRECTION_NORTH, DIRECTION_WEST}, // south-east corner wall
		{DIRECTION_NORTH, DIRECTION_EAST}, // south-west corner wall
		{DIRECTION_SOUTH, DIRECTION_EAST}, // north-west corner wall
	};
	
	resources::SpriteSheetInfo info = this->container->getSpriteInfo(position + glm::ivec3(0, 0, 1));
	if(
		(
			this->container->getTile(position + glm::ivec3(0, 0, 1)) != 0
			&& info.wall == resources::NO_WALL
		)
		|| this->container->positionToCharacter.find(position + glm::ivec3(0, 0, 1)) != this->container->positionToCharacter.end()
	) {
		return false;
	}

	// test for invalid walls
	if(info.wall != resources::NO_WALL) {
		for(unsigned int i = 0; i < 2; i++) {
			if(invalidDirectionsForWall[lsb(info.wall)][i] == direction) {
				return false;
			}
		}
	}

	return true;
}
