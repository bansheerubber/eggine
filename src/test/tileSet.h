#pragma once

#include <tsl/robin_set.h>
#include <glm/vec2.hpp>

#include "../basic/gameObject.h"

class TileSet: public GameObject {
	public:
		TileSet();
		~TileSet();
		
		bool add(glm::uvec2 position);
		bool remove(glm::uvec2 position);
		void clear();
		bool has(glm::uvec2 position);
	
	protected:
		tsl::robin_set<unsigned int> set;
		unsigned int index(glm::uvec2 &position);
};

namespace es {
	void defineTileSet();
	esEntryPtr TileSet__add(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__remove(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__clear(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__has(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};
