#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "../basic/gameObject.h"
#include "../basic/renderContext.h"
#include "../util/sortedArray.h"
#include "tileMath.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

int layerTilesCompare(class OverlappingTile** a, class OverlappingTile** b);

class Layer : public GameObject {
	friend class Chunk;
	
	public:
		Layer(class Chunk* chunk);

		// ## game_object_definitions Layer

		void render(double deltaTime, RenderContext &context);
		void renderOccluded(double deltaTime, RenderContext &context);
		void add(OverlappingTile* tile);
		void update(OverlappingTile* tile);
		void remove(OverlappingTile* tile);

	private:
		class Chunk* chunk = nullptr;
		SortedArray<class OverlappingTile*> tiles = SortedArray<class OverlappingTile*>(layerTilesCompare);
		DynamicArray<glm::vec2> offsets = DynamicArray<glm::vec2>(4); // CPU storage for tile offsets
		DynamicArray<int> textureIndices = DynamicArray<int>(4); // CPU storage for tile texture indices
		DynamicArray<glm::vec4> colors = DynamicArray<glm::vec4>(4); // CPU storage for color offsets

		render::VertexBuffer* buffers[3];
		render::VertexAttributes* attributes;
		bool needsUpdate = false;
		bool needsSort = false;

		void rebuildBuffers();
		void updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation);
};
