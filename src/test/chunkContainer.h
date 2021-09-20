#pragma once

#include <vector>

#include "chunk.h"
#include "../util/dynamicArray.h"
#include "../basic/renderObject.h"

using namespace std;

void initChunk(class ChunkContainer* container, class Chunk* chunk);

class ChunkContainer : public RenderObject {
	friend class Engine;
	
	public:
		ChunkContainer();

		// ## game_object_definitions ChunkContainer

		void addOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);
		void setOverlappingTileChunk(OverlappingTile* tile);
		
		void addChunk(glm::uvec2 position);
		Chunk& getChunk(size_t index);
		size_t getChunkCount();

		void render(double deltaTime, RenderContext &context);

		static GLuint Shaders[];
		static GLuint Uniforms[];
		static GLuint ShaderProgram;

		unsigned int size = 0;
	
	private:
		vector<class OverlappingTile*> overlappingTiles;
		DynamicArray<Chunk, ChunkContainer> renderOrder = DynamicArray<Chunk, ChunkContainer>(this, 8, initChunk, nullptr);
};
