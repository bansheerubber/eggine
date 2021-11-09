#pragma once

#define GLM_GTX_hash

#include <glm/gtx/hash.hpp>
#include <tsl/robin_map.h>
#include <glm/vec3.hpp>
#include <vector>

#include "chunk.h"
#include "../util/dynamicArray.h"
#include "neighbors.h"
#include "map.h"
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
	esEntryPtr ChunkContainer__getSelectedCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__setTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr tileToScreen(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class ChunkContainer : public RenderObject {
	friend class Character;
	friend TileNeighborIterator;
	friend esEntryPtr es::ChunkContainer__getCharacter(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::ChunkContainer__getPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__getSelectedCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__setTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend class Engine;
	
	public:
		ChunkContainer();
		~ChunkContainer();

		// ## game_object_definitions ChunkContainer
		unsigned int size = 0;
		Map map = Map(this);

		void selectCharacter(class Character* character);
		void selectTile(glm::ivec3 position, bool browsing);
		
		Chunk& addChunk(glm::uvec2 position);
		Chunk& getChunk(size_t index);
		size_t getChunkCount();

		void render(double deltaTime, RenderContext &context);

		void commit();

		void onBind(string &bind, binds::Action action);
		void onAxis(string &bind, double value);

		class Team* getPlayerTeam();

		bool isValidTilePosition(glm::ivec3 position);

		void setTile(glm::ivec3 position, int texture);
		int getTile(glm::ivec3 position);
		resources::SpriteSheetInfo getSpriteInfo(glm::ivec3 position);
		TileNeighborIterator getNeighbors(glm::ivec3 position);

		static render::Program* Program;
		static resources::SpriteSheet* Image;

		static render::VertexBuffer* Vertices;
		static render::VertexBuffer* UVs;
		static render::VertexBuffer* Colors;
	
	private:
		class OverlappingTile* tileSelectionSprite = nullptr;
		class OverlappingTile* characterSelectionSprite = nullptr;
		class Character* selectedCharacter = nullptr;
		class Team* playerTeam = nullptr;
		DynamicArray<Chunk, ChunkContainer> renderOrder = DynamicArray<Chunk, ChunkContainer>(this, 0, initChunk, nullptr);

		tsl::robin_map<glm::uvec3, class Character*> positionToCharacter;

		void updateCharacterPosition(class Character* character, glm::uvec3 newPosition);
		glm::ivec3 findCandidateSelectedTile(glm::vec2 screenSpace);
		void rightClickTile(glm::ivec3 position);
		void hoverTile(glm::ivec3 position);

		static constexpr glm::vec2 VerticesSource[4] = {
			glm::vec2(-0.501f,  1.001f),
			glm::vec2(-0.501f, -1.001f),
			glm::vec2(0.501f, 1.001f),
			glm::vec2(0.501f, -1.001f)
		};

		static constexpr glm::vec2 UVsSource[4] = {
			glm::vec2(0.0f, 0.0f),
			glm::vec2(0.0f, 1.0),
			glm::vec2(1.0, 0.0f),
			glm::vec2(1.0, 1.0)
		};

		static constexpr glm::vec4 ColorsSource[4] = {
			glm::vec4(1, 1, 1, 1),
			glm::vec4(1, 1, 1, 1),
			glm::vec4(1, 1, 1, 1),
			glm::vec4(1, 1, 1, 1)
		};
};
