#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../basic/renderObject.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

class OverlappingTile : public RenderObject {	
	public:
		OverlappingTile(class ChunkContainer* container);
		~OverlappingTile();

		OverlappingTile* setPosition(glm::uvec3 position);
		glm::uvec3 getPosition();

		OverlappingTile* setTexture(unsigned int index);
		OverlappingTile* setColor(glm::vec4 color);
		glm::vec4 getColor();
		OverlappingTile* setZIndex(unsigned int zIndex);
		unsigned int getZIndex();

		void render(double deltaTime, RenderContext &context);

	protected:
		class ChunkContainer* container = nullptr;
		
		render::VertexBuffer* vertexBuffers[2];
		render::VertexAttributes* vertexAttributes;
		int textureIndex = 3;
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		unsigned int zIndex = 0;
		
		glm::uvec3 position = glm::uvec3(0, 0, 0);
		glm::vec2 screenSpacePosition = glm::vec2(0, 0);
		class Chunk* chunk = nullptr;
};
