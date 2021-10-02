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

		bool move(glm::uvec3 position);
		bool moveTest(glm::uvec3 position);
};

namespace ts {
	void defineCharacter();
	tsEntryPtr Character__getDestinations(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
};
