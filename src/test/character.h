#pragma once

#include <glm/vec2.hpp>

#include "../engine/engine.h"
#include "../basic/gameObject.h"
#include "overlappingTile.h"
#include "tileSet.h"

class Character: public OverlappingTile {
	public:
		Character();
		~Character();

		glm::uvec2 position = glm::uvec2(0, 0);
		TileSet destinations; // TODO move to unit class

		bool move(glm::uvec2 position);
		bool moveTest(glm::uvec2 position);
};

namespace ts {
	void defineCharacter();
	tsEntryPtr Character__getDestinations(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
};
