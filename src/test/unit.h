#pragma once

#define GLM_GTX_hash

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

#include "character.h"
#include "tileSet.h"

namespace es {
	void defineUnit();
	esEntryPtr Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class Unit: public Character {
	friend esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	
	public:
		Unit(class ChunkContainer* container);
		~Unit();

	private:
		TileSet destinations;
		unsigned int moves = 5;

		void calculateDestinations(glm::ivec3 tilePosition = glm::ivec3(-1, -1, -1), unsigned int move = 0);
};
