#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "../basic/gameObject.h"
#include "overlappingTile.h"
#include "../basic/renderContext.h"
#include "../util/sortedArray.h"
#include "tileMath.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

class Layer : public GameObject {
	friend class Chunk;
	
	public:
		Layer(class Chunk* chunk);

		// ## game_object_definitions Layer

		void render(double deltaTime, RenderContext &context);
		void renderXRay(double deltaTime, RenderContext &context);
		void add(OverlappingTile* tile);
		void update(OverlappingTile* tile);
		void remove(OverlappingTile* tile);

	private:
		class Chunk* chunk = nullptr;
		SortedArray<class OverlappingTile*> tiles = SortedArray<class OverlappingTile*>();

		render::VertexBuffer* buffers[3];
		render::VertexBuffer* xrayBuffers[4];
		render::VertexAttributes* attributes;
		render::VertexAttributes* xrayAttributes;
		bool needsUpdate = false;
		bool needsSort = false;

		unsigned int xrayCount = 0;

		// temporary storage for buffers
		static DynamicArray<glm::vec3> Offsets;
		static DynamicArray<int> TextureIndices;
		static DynamicArray<glm::vec4> Colors;

		static DynamicArray<glm::vec3> XRayOffsets;
		static DynamicArray<int> XRayTextureIndices;
		static DynamicArray<glm::vec4> XRayColors;
		static DynamicArray<int> XRayEnabled;

		constexpr static int XRay = 2;

		void rebuildBuffers();
		void updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation);
};
