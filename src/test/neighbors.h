#pragma once

#include <glm/vec3.hpp>

enum NeighborDirection {
	INVALID_DIRECTION = 0,
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST
};

class TileNeighborIterator {
	public:
		TileNeighborIterator(glm::ivec3 position);
		
		glm::ivec3& value();
		bool atEnd();
		bool operator==(const TileNeighborIterator &other);
		TileNeighborIterator& operator++();
	
	private:
		int index = 0;
		glm::ivec3 position;
		glm::ivec3 foundPosition;
		class ChunkContainer* container;

		void iterateTile();
		bool testTile(glm::ivec3 position, NeighborDirection direction);
};
