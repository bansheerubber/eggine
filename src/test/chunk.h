#pragma once

#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <tsl/robin_map.h>

#include "../engine/debug.h"
#include "../basic/instancedRenderObjectContainer.h"
#include "../util/minHeap.h"
#include "../basic/pngImage.h"
#include "../basic/renderContext.h"
#include "../basic/renderObject.h"

class Tile;

struct OverlappingTileWrapper {
	unsigned int index;
	class OverlappingTile* tile;

	bool operator<(const OverlappingTileWrapper &other) {
		return this->index < other.index;
	}

	bool operator>(const OverlappingTileWrapper &other) {
		return this->index > other.index;
	}
};

void initOverlappingTileWrapper(class Chunk* chunk, OverlappingTileWrapper** tile);

class Chunk : public InstancedRenderObjectContainer<Tile> {
	friend class ChunkContainer;
	friend class OverlappingTile;
	
	public:
		Chunk(glm::uvec2 position);

		// ## game_object_definitions Chunk

		void render(double deltaTime, RenderContext &context) {};
		void renderChunk(double deltaTime, RenderContext &context);

		glm::vec2* offsets = nullptr;
		int* textureIndices = nullptr;

		glm::uvec2 position = glm::uvec2(0, 0);
		glm::vec2 screenSpacePosition;
		unsigned int height = 5;

		void addOverlappingTile(class OverlappingTile* tile);
		void removeOverlappingTile(class OverlappingTile* tile);

		static constexpr unsigned int Size = 25;
		static GLuint Texture;
	
	protected:
		MinHeap<OverlappingTileWrapper*, Chunk> overlappingTiles = MinHeap<OverlappingTileWrapper*, Chunk>(this, initOverlappingTileWrapper, nullptr);
		
		void buildDebugLines();
		void defineBounds();

		#ifdef EGGINE_DEBUG
		bool isCulled = false;
		int drawCalls = 0;
		#endif
		
		GLuint vertexBufferObjects[1];
		GLuint vertexArrayObject;

		class Line* debugLine = nullptr;

		tsl::robin_map<int, tsl::robin_map<int, int>> tiles;

		double top = 0, right = 0, bottom = 0, left = 0;

		static PNGImage* Image;
		static glm::vec2 Offsets[];
		static GLuint VertexBufferObjects[];

		static constexpr float Vertices[8] = {
			-0.5f,  1.0f,
			-0.5f, -1.0f,
			0.5f, 1.0f,
			0.5f, -1.0f
		};

		static constexpr float UVs[8] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
};
