#pragma once

#include <glm/vec3.hpp>

class TileNeighborIterator {
	public:
		TileNeighborIterator(class ChunkContainer* container, glm::ivec3 position);
		
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
		bool testTile(glm::ivec3);
};
