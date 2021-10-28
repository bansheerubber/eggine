#pragma once

#include <vector>

#include "chunk.h"
#include "../util/dynamicArray.h"
#include "../renderer/program.h"
#include "../basic/renderObject.h"
#include "../resources/spriteSheet.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk* chunk);

class ChunkContainer : public RenderObject {
	friend class Engine;
	
	public:
		ChunkContainer();
		~ChunkContainer();

		// ## game_object_definitions ChunkContainer

		void addOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);
		void setOverlappingTileChunk(OverlappingTile* tile);
		
		void addChunk(glm::uvec2 position);
		Chunk& getChunk(size_t index);
		size_t getChunkCount();

		void render(double deltaTime, RenderContext &context);

		void commit();

		void onBindPress(string &bind);
		void onAxis(string &bind, double value);

		unsigned int size = 0;

		// static class Shader* Program;
		static render::Program* Program;
		static resources::SpriteSheet* Image;
	
	private:
		class OverlappingTile* tileSelectionSprite;
		DynamicArray<Chunk, ChunkContainer> renderOrder = DynamicArray<Chunk, ChunkContainer>(this, 8, initChunk, nullptr);
};
