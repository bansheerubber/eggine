#pragma once

#include <glm/vec3.hpp>

enum NeighborDirection {
	INVALID_DIRECTION = 0,
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST,
	DIRECTION_UP,
	DIRECTION_DOWN,
};

inline NeighborDirection flipDirection(NeighborDirection direction) {
	switch(direction) {
		case DIRECTION_NORTH: {
			return DIRECTION_SOUTH;
		}

		case DIRECTION_EAST: {
			return DIRECTION_WEST;
		}

		case DIRECTION_SOUTH: {
			return DIRECTION_NORTH;
		}

		case DIRECTION_WEST: {
			return DIRECTION_EAST;
		}

		case DIRECTION_UP: {
			return DIRECTION_DOWN;
		}

		case DIRECTION_DOWN: {
			return DIRECTION_UP;
		}
		
		default: {
			return INVALID_DIRECTION;
		}
	}
}

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
