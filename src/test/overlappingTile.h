#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../basic/renderObject.h"

class OverlappingTile : public RenderObject {	
	public:
		OverlappingTile();
		~OverlappingTile();

		void setPosition(glm::uvec3 position);
		glm::uvec3 getPosition();
		void setChunk(class Chunk* chunk);

		void render(double deltaTime, RenderContext &context);

	private:
		GLuint vertexBufferObjects[2];
		GLuint vertexArrayObject;
		int textureIndex = 3;
		
		glm::uvec3 position = glm::uvec3(0, 0, 0);
		glm::lowp_vec2 screenSpacePosition = glm::vec2(0, 0);
		class Chunk* chunk = nullptr;
};
