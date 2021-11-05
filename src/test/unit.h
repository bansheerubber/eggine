#pragma once

#define GLM_GTX_hash

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

#include "character.h"
#include "tileSet.h"

namespace es { // order = 4
	void defineUnit();
	esEntryPtr Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__setMoves(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class Unit: public Character {
	friend esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__setMoves(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	
	public:
		Unit(class ChunkContainer* container, bool createReference = true);
		~Unit();

		void move(glm::ivec3 position);

	private:
		TileSet destinations;
		unsigned int moves = 5;

		struct {
			unsigned int moves;
			glm::uvec3 position;
		} lastDestinationsCalculation;

		void calculateDestinations();
};
