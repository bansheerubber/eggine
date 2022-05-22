#pragma once

#define GLM_ENABLE_EXPERIMENTAL
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
#include "tileMath.h"

void initChunk(class ChunkContainer* container, class Chunk** chunk);

namespace es {
	void defineChunkContainer();
	esEntryPtr getChunkContainer(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr ChunkContainer__getCharacter(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr ChunkContainer__selectCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__setPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__setEnemyTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getEnemyTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getSelectedCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__setTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getWall(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__setRotation(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr ChunkContainer__getRotation(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr tileToScreen(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class ChunkContainer : public RenderObject {
	friend class Character;
	friend esEntryPtr es::ChunkContainer__getCharacter(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::ChunkContainer__getPlayerTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__getEnemyTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__getSelectedCharacter(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__setTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__getTile(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend esEntryPtr es::ChunkContainer__getWall(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	friend class Engine;
	
	public:
		ChunkContainer();
		~ChunkContainer();

		// ## game_object_definitions ChunkContainer
		Map map = Map(this);

		void selectCharacter(class Character* character);
		class Character* getSelectedCharacter();
		void selectTile(glm::ivec3 position, bool browsing, bool controller);

		void setSize(unsigned int size);
		unsigned int getSize();

		void setRotation(tilemath::Rotation rotation);
		tilemath::Rotation getRotation();
		
		Chunk* addChunk(glm::uvec2 position);
		Chunk* getChunk(uint64_t index);
		uint64_t getChunkCount();

		void render(double deltaTime, RenderContext &context);

		void commit();

		void onBind(std::string &bind, binds::Action action);
		void onAxis(std::string &bind, double value);

		void setPlayerTeam(class Team*);
		class Team* getPlayerTeam();

		void setEnemyTeam(class Team*);
		class Team* getEnemyTeam();

		bool isValidTilePosition(glm::uvec3 position);

		void setTile(glm::ivec3 position, int texture);
		resources::SpriteSheetInfo getSpriteInfo(glm::ivec3 position, bool original = false);
		TileNeighborIterator getNeighbors(glm::ivec3 position);
		class Character* getCharacter(glm::ivec3 position);

		static render::Program* Program;
		static resources::SpriteSheet* Image;

		static render::VertexBuffer* Vertices;
		static render::VertexBuffer* UVs;
		static render::VertexBuffer* Colors;
		static render::VertexBuffer* XRay;

		unsigned int timer = 0;
	
	private:
		unsigned int size = 0;
		tilemath::Rotation rotation = tilemath::ROTATION_0_DEG;
		
		class OverlappingTile* tileSelectionSprite = nullptr;
		class OverlappingTile* characterSelectionSprite = nullptr;
		class Character* selectedCharacter = nullptr;
		class Team* playerTeam = nullptr;
		class Team* enemyTeam = nullptr;
		DynamicArray<Chunk*, ChunkContainer> renderOrder = DynamicArray<Chunk*, ChunkContainer>(this, 0, initChunk, nullptr);

		tsl::robin_map<glm::uvec3, class Character*> positionToCharacter;

		void updateCharacterPosition(class Character* character, glm::uvec3 newPosition);
		glm::vec3 screenToTile(glm::vec2 screenSpace);
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

		static constexpr int XRaySource = 0;
};
