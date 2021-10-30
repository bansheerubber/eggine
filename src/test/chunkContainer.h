#pragma once

#define GLM_GTX_hash

#include <glm/gtx/hash.hpp>
#include <tsl/robin_map.h>
#include <glm/vec3.hpp>
#include <vector>

#include "chunk.h"
#include "../util/dynamicArray.h"
#include "../renderer/program.h"
#include "../basic/renderObject.h"
#include "../resources/spriteSheet.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk* chunk);

namespace es {
	void defineChunkContainer();
	esEntryPtr getChunkContainer(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr ChunkContainer__getCharacter(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr ChunkContainer__selectCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr tileToScreen(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class ChunkContainer : public RenderObject {
	friend class Character;
	friend esEntryPtr es::ChunkContainer__getCharacter(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::ChunkContainer__getPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend class Engine;
	
	public:
		ChunkContainer();
		~ChunkContainer();

		// ## game_object_definitions ChunkContainer

		void addOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);
		void setOverlappingTileChunk(OverlappingTile* tile);

		void selectCharacter(class Character* character);
		void selectTile(glm::uvec3 position, bool browsing);
		
		void addChunk(glm::uvec2 position);
		Chunk& getChunk(size_t index);
		size_t getChunkCount();

		void render(double deltaTime, RenderContext &context);

		void commit();

		void onBind(string &bind, binds::Action action);
		void onAxis(string &bind, double value);

		class Team* getPlayerTeam();

		unsigned int size = 0;

		// static class Shader* Program;
		static render::Program* Program;
		static resources::SpriteSheet* Image;
	
	private:
		class OverlappingTile* tileSelectionSprite;
		class OverlappingTile* characterSelectionSprite;
		class Character* selectedCharacter;
		class Team* playerTeam;
		DynamicArray<Chunk, ChunkContainer> renderOrder = DynamicArray<Chunk, ChunkContainer>(this, 8, initChunk, nullptr);
		
		tsl::robin_map<glm::uvec3, class Character*> positionToCharacter;

		void updateCharacterPosition(class Character* character, glm::uvec3 newPosition);
};
