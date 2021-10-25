#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../basic/renderObject.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

class OverlappingTile : public RenderObject {	
	public:
		OverlappingTile(class ChunkContainer* container);
		~OverlappingTile();

		void setPosition(glm::uvec3 position);
		glm::uvec3 getPosition();

		void render(double deltaTime, RenderContext &context);

	protected:
		class ChunkContainer* container = nullptr;
		
		render::VertexBuffer* vertexBuffers[2];
		render::VertexAttributes* vertexAttributes;
		int textureIndex = 3;
		
		glm::uvec3 position = glm::uvec3(0, 0, 0);
		glm::vec2 screenSpacePosition = glm::vec2(0, 0);
		class Chunk* chunk = nullptr;
};
