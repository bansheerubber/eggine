#pragma once

#include "overlappingTile.h"
#include "../basic/renderContext.h"
#include "../basic/renderObject.h"

namespace es { // order = 2
	void defineInterweavedTile();
	void InterweavedTile__constructor(esObjectWrapperPtr wrapper);
	void InterweavedTile__deconstructor(esObjectWrapperPtr wrapper);
};

class InterweavedTile: public OverlappingTile {
	public:
		InterweavedTile(class ChunkContainer* container, bool createReference = true);
		~InterweavedTile();

		OverlappingTile* setPosition(glm::uvec3 position);
		OverlappingTile* setTexture(unsigned int index);
		OverlappingTile* setColor(glm::vec4 color);
		OverlappingTile* setZIndex(unsigned int zIndex);

		void render(double deltaTime, RenderContext &context);
	
	protected:
		render::VertexBuffer* vertexBuffers[3];
		render::VertexAttributes* vertexAttributes;
};
