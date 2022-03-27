#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTX_hash

#include <glm/gtx/hash.hpp>
#include <tsl/robin_set.h>
#include <tsl/robin_map.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../basic/gameObject.h"

enum AdjacencyBit {
	ADJACENT_NORTH = 1,
	ADJACENT_EAST = 2,
	ADJACENT_SOUTH = 4,
	ADJACENT_WEST = 8,
};

typedef unsigned int AdjacencyBitmask;

class TileSet: public GameObject {
	public:
		TileSet();
		~TileSet();

		// ## game_object_definitions TileSet
		
		bool add(glm::uvec3 position);
		bool remove(glm::uvec3 position);
		void clear();
		bool has(glm::uvec3 position);
		unsigned int size();
		AdjacencyBitmask adjacency(glm::ivec3 position);
		void showDots();
		void showBorder(glm::vec4 color = glm::vec4(1, 1, 1, 1));
		void hideBorder();
		TileSet* pathfind(glm::ivec3 start, glm::ivec3 end);

		glm::uvec3 getCameFrom(glm::uvec3 position);
		glm::uvec3 getWentTo(glm::uvec3 position);
	
	protected:
		tsl::robin_set<glm::uvec3> set;
		tsl::robin_set<class OverlappingTile*> border;

		tsl::robin_map<glm::uvec3, glm::uvec3> cameFrom;
		tsl::robin_map<glm::uvec3, glm::uvec3> wentTo;
};

namespace es {
	void defineTileSet();
	esEntryPtr TileSet__add(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__remove(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__clear(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__has(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__size(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__showBorder(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__showDots(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__hideBorder(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__getCameFrom(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr TileSet__getWentTo(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};
