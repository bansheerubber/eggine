#pragma once

#include "overlappingTile.h"
#include "../basic/renderContext.h"
#include "../basic/renderObject.h"
#include "tileMath.h"

namespace es { // order = 2
	void defineInterweavedTile();
	void InterweavedTile__constructor(esObjectWrapperPtr wrapper);
	void InterweavedTile__deconstructor(esObjectWrapperPtr wrapper);
};

class InterweavedTile: public OverlappingTile {
	friend class Chunk;

	public:
		InterweavedTile(bool createReference = true);
		virtual ~InterweavedTile();

		// ## game_object_definitions InterweavedTile

		OverlappingTile* setPosition(glm::uvec3 position);
		OverlappingTile* setTexture(unsigned int index);
		OverlappingTile* setColor(glm::vec4 color);
		OverlappingTile* setZIndex(unsigned int zIndex);

		void render(double deltaTime, RenderContext &context);
		void renderXRay(double deltaTime, RenderContext &context);
	
	protected:
		render::VertexBuffer* vertexBuffers[4];
		render::VertexAttributes* vertexAttributes;

		render::VertexBuffer* xrayVertexBuffer;
		render::VertexAttributes* xrayVertexAttributes;

		void updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation);
};
