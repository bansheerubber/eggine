#pragma once

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
};
