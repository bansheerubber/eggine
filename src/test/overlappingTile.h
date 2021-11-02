#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../basic/gameObject.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

class OverlappingTile : public GameObject {	
	friend class Layer;
	
	public:
		OverlappingTile(class ChunkContainer* container);
		~OverlappingTile();

		OverlappingTile* setPosition(glm::uvec3 position);
		glm::uvec3 getPosition();

		OverlappingTile* setTexture(unsigned int index);
		int getTexture();
		OverlappingTile* setColor(glm::vec4 color);
		glm::vec4 getColor();
		OverlappingTile* setZIndex(unsigned int zIndex);
		unsigned int getZIndex();

		class ChunkContainer* getContainer();
		class Layer* getLayer();

	protected:
		class ChunkContainer* container = nullptr;
		
		int textureIndex = 3;
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		unsigned int zIndex = 0;
		
		glm::uvec3 position = glm::uvec3(0, 0, 0);
		glm::vec2 screenSpacePosition = glm::vec2(0, 0);
		class Layer* layer =  nullptr;
		class Chunk* chunk = nullptr;
};
