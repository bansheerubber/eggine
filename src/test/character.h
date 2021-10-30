#pragma once

#include <glm/vec2.hpp>

#include "../engine/engine.h"
#include "../basic/gameObject.h"
#include "overlappingTile.h"
#include "tileSet.h"

class Character: public OverlappingTile {
	public:
		Character(class ChunkContainer* container);
		~Character();

		TileSet destinations; // TODO move to unit class

		void setPosition(glm::uvec3 position);
		bool move(glm::uvec3 position);
		bool moveTest(glm::uvec3 position);
};

namespace es {
	void defineCharacter();
	esEntryPtr Character__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Character__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Character__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};
