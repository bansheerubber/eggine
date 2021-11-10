#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../basic/gameObject.h"
#include "tileMath.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

namespace es { // order = 1
	void defineOverlappingTile();
	void OverlappingTile__constructor(esObjectWrapperPtr wrapper);
	void OverlappingTile__deconstructor(esObjectWrapperPtr wrapper);
	esEntryPtr OverlappingTile__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__setTexture(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__getTexture(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__setColor(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__getColor(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__setZIndex(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	esEntryPtr OverlappingTile__getZIndex(esEnginePtr esEngine, unsigned int argc, esEntry* args);
};

class OverlappingTile : public GameObject {	
	friend class Layer;
	
	public:
		OverlappingTile(class ChunkContainer* container, bool createReference = true);
		~OverlappingTile();

		virtual OverlappingTile* setPosition(glm::uvec3 position);
		glm::uvec3 getPosition();

		virtual OverlappingTile* setTexture(unsigned int index);
		int getTexture();
		virtual OverlappingTile* setColor(glm::vec4 color);
		glm::vec4 getColor();
		virtual OverlappingTile* setZIndex(unsigned int zIndex);
		unsigned int getZIndex();

		class ChunkContainer* getContainer();
		class Layer* getLayer();

	protected:
		class ChunkContainer* container = nullptr;
		
		int textureIndex = 4;
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		unsigned int zIndex = 0;
		
		glm::uvec3 position = glm::uvec3(0, 0, 0);
		glm::vec2 screenSpacePosition = glm::vec2(0, 0);
		class Layer* layer =  nullptr;
		class Chunk* chunk = nullptr;

		void updateRotation(tilemath::Rotation rotation);
};
