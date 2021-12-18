#pragma once

#define GLM_GTX_hash

#include <glm/gtx/hash.hpp>
#include <glm/vec3.hpp>

#include "character.h"
#include "healthbar.h"
#include "tileSet.h"

namespace es { // order = 4
	void defineUnit();
	esEntryPtr Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__getSprintDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__getPath(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__setMoves(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__kill(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__setHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__setMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__addHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__addMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__getHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr Unit__getMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class Unit: public Character {
	friend esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__getSprintDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__getPath(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__setMoves(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__kill(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__setHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__setMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__addHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__addMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__getHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::Unit__getMaxHealth(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend class Team;
	
	public:
		Unit(class ChunkContainer* container, bool createReference = true);
		~Unit();

		// ## game_object_definitions Unit

		void move(glm::ivec3 position);
		OverlappingTile* setPosition(glm::uvec3 position);

		void kill();
		void setHealth(int health);
		void setMaxHealth(int maxHealth);
		void addHealth(int health);
		void addMaxHealth(int maxHealth);
		int getHealth();
		int getMaxHealth();

	private:
		class Team* team = nullptr;
		TileSet destinations;
		TileSet sprintDestinations;
		TileSet* path = nullptr;
		unsigned int moves = 5;
		unsigned int sprintMoves = 15;

		int maxHealth = 1;
		int health = 1;
		Healthbar healthbar;

		void calculateDestinations(TileSet &destinations, unsigned int moves);
		TileSet* getPath(glm::ivec3 end);
};
