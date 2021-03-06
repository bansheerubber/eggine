#pragma once

#include <glm/vec2.hpp>

#include "../engine/engine.h"
#include "../basic/gameObject.h"
#include "interweavedTile.h"
#include "../network/networkMacros.h"
#include "../network/types.h"
#include "../network/stream.h"
#include "../basic/remoteObject.h"
#include "../resources/spriteSheet.h"
#include "tileSet.h"

class Character: public InterweavedTile, public network::RemoteObject {
	public:
		Character(bool createReference = true);
		~Character();

		// ## game_object_definitions Character
		// ##1 remote_object_definitions Character

		virtual OverlappingTile* setPosition(glm::uvec3 position);
		bool move(glm::uvec3 position);
		bool moveTest(glm::uvec3 position);

		void setAppearance(unsigned int texture);
		void setDirection(resources::SpriteFacing direction);
	
	protected:
		NP_PROPERTY(NP_VECTOR, Character::setPosition);
		glm::uvec3 position;

		NP_PROPERTY(NP_NUMBER, Character::setAppearance);
		unsigned int appearance = 4;

		resources::SpriteFacing direction = resources::FACING_NORTH;
};

namespace es { // order = 3
	void defineCharacter();
	esEntryPtr Character__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Character__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Character__setDirection(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};
