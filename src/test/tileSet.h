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

namespace ts {
	void defineTileSet();
	tsEntryPtr TileSet__add(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
	tsEntryPtr TileSet__remove(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
	tsEntryPtr TileSet__clear(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
	tsEntryPtr TileSet__has(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
};
