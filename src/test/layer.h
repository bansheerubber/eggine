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

		render::VertexBuffer* buffers[3];
		render::VertexBuffer* occludedBuffers[4];
		render::VertexAttributes* attributes;
		render::VertexAttributes* occludedAttributes;
		bool needsUpdate = false;
		bool needsSort = false;

		unsigned int occludedCount = 0;

		// temporary storage for buffers
		static DynamicArray<glm::vec3> Offsets;
		static DynamicArray<int> TextureIndices;
		static DynamicArray<glm::vec4> Colors;

		static DynamicArray<glm::vec3> OccludedOffsets;
		static DynamicArray<int> OccludedTextureIndices;
		static DynamicArray<glm::vec4> OccludedColors;
		static DynamicArray<int> OccludedEnabled;

		constexpr static int Occluded = 2;

		void rebuildBuffers();
		void updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation);
};
