#include "chunk.h"

#include <random>

#include "../basic/camera.h"
#include "chunkContainer.h"
#include "../util/doubleDimension.h"
#include "../engine/engine.h"
#include "layer.h"
#include "../basic/line.h"
#include "overlappingTile.h"
#include "../resources/resourceManager.h"
#include "tileMath.h"

glm::vec2 Chunk::OffsetsSource[Chunk::Size * Chunk::Size * Chunk::MaxHeight];
render::VertexBuffer* Chunk::Offsets = nullptr;

Chunk::Chunk(ChunkContainer* container) : InstancedRenderObjectContainer(false) {
	this->container = container;
	
	if(Chunk::Offsets == nullptr) {
		// pre-calculate offsets
		{
			for(unsigned i = 0; i < Size * Size; i++) {
				for(unsigned z = 0; z < Chunk::MaxHeight; z++) {
					glm::ivec2 coordinate = tilemath::indexToCoordinate(i, Size);
					Chunk::OffsetsSource[i + z * Size * Size] = tilemath::tileToScreen(glm::vec3(coordinate, z));
				}
			}

			Chunk::Offsets = new render::VertexBuffer(&engine->renderWindow);
			Chunk::Offsets->setData(&Chunk::OffsetsSource[0], sizeof(glm::vec2) * Chunk::Size * Chunk::Size * Chunk::MaxHeight, alignof(glm::vec2));
		}
	}

	// this->height = ((double)rand() / (RAND_MAX)) * 10 + 1;
	this->height = 5;

	this->vertexBuffer = new render::VertexBuffer(&engine->renderWindow);
	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);

	this->textureIndices = new int[Size * Size * Chunk::MaxHeight];
	for(unsigned i = 0; i < Size * Size * Chunk::MaxHeight; i++) {
		this->textureIndices[i] = 0;
	}

	for(unsigned i = 0; i < Size * Size * this->height; i++) {
		this->textureIndices[i] = 3;
	}
	
	// load vertices
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load uvs
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::UVs, 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load offsets
	{
		this->vertexAttributes->addVertexAttribute(Chunk::Offsets, 2, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 1);
	}

	// load texture indices
	{
		this->vertexBuffer->setData(this->textureIndices, sizeof(int) * Chunk::Size * Chunk::Size * Chunk::MaxHeight, alignof(int));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffer, 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}

	// load colors
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::Colors, 4, 4, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec4), 0);
	}

	this->defineBounds();
}

Chunk::~Chunk() {
	delete this->textureIndices;
	if(this->debugLine != nullptr) {
		delete this->debugLine;
	}
}

void Chunk::setPosition(glm::uvec2 position) {
	this->position = position;
	this->screenSpacePosition = tilemath::tileToScreen(glm::vec3((unsigned int)Size * this->position, 0.0));
	this->defineBounds();
}

glm::uvec2& Chunk::getPosition() {
	return this->position;
}

void Chunk::defineBounds() {
	glm::vec2 bias(-0.5, -(32.0 / 2.0 + 39.0 * 2.0 + 2) / 128.0);

	glm::vec2 top = tilemath::tileToScreen(glm::vec3(Size, 0, this->height)) + this->screenSpacePosition + bias;
	glm::vec2 right = tilemath::tileToScreen(glm::vec3(Size, Size, 0)) + this->screenSpacePosition + bias;
	glm::vec2 bottom = tilemath::tileToScreen(glm::vec3(0, Size, 0)) + this->screenSpacePosition + bias;
	glm::vec2 left = tilemath::tileToScreen(glm::vec3(0, 0, 0)) + this->screenSpacePosition + bias;

	this->left = left.x;
	this->right = right.x;
	this->top = top.y;
	this->bottom = bottom.y;
}

void Chunk::buildDebugLines() {
	this->debugLine = new Line();
	this->debugLine->addPosition(glm::vec2(this->left, this->bottom));
	this->debugLine->addPosition(glm::vec2(this->right, this->bottom));
	this->debugLine->addPosition(glm::vec2(this->right, this->top));
	this->debugLine->addPosition(glm::vec2(this->left, this->top));
	this->debugLine->addPosition(glm::vec2(this->left, this->bottom));
	this->debugLine->commit();
}

void Chunk::renderChunk(double deltaTime, RenderContext &context) {
	#ifdef EGGINE_DEBUG
	this->drawCalls = 0;
	#endif

	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 chunkScreenSpace;
		float spritesheetWidth;
		float spritesheetHeight;
		float spriteWidth;
		float spriteHeight;
		float spritesOnRow;
	} vb;
	vb.projection = context.camera->projectionMatrix;
	vb.chunkScreenSpace = this->screenSpacePosition;
	vb.spritesheetWidth = 1057;
	vb.spritesheetHeight = 391;
	vb.spriteWidth = 64.0f;
	vb.spriteHeight = 128.0f;
	vb.spritesOnRow = floor(vb.spritesheetWidth / vb.spriteWidth);

	struct FragmentBlock {
		glm::vec4 color;
	} fb;
	fb.color = glm::vec4(1, 1, 1, 1);

	ChunkContainer::Program->bindUniform("vertexBlock", &vb, sizeof(vb));
	
	Camera* camera = context.camera;
	if(!(
		camera->left > this->right
		|| camera->right < this->left
		|| camera->top < this->bottom
		|| camera->bottom > this->top
	)) {
		// try to draw as many layers at once as we can
		unsigned int start = 0;
		unsigned int end = 0;
		bool rendered = false;
		for(unsigned int i = 0; i < this->height; i++) {
			end = i;
			rendered = false;

			if(this->getLayer(i) != nullptr) { // check if we need to stop and render the layer
				this->vertexAttributes->bind();
				engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, start * Chunk::Size * Chunk::Size, (end - start + 1) * Chunk::Size * Chunk::Size);
				#ifdef EGGINE_DEBUG
				this->drawCalls++;
				#endif

				this->getLayer(i)->render(deltaTime, context);

				start = i + 1;
				end = i;
				rendered = true;
			}
		}

		if(!rendered) { // render remaining tiles at top of the chunk
			this->vertexAttributes->bind();
			engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, start * Chunk::Size * Chunk::Size, (end - start + 1) * Chunk::Size * Chunk::Size);
			#ifdef EGGINE_DEBUG
			this->drawCalls++;
			#endif
		}

		// render remaining overlapping tiles
		for(unsigned int i = end; i <= this->maxLayer; i++) {
			if(this->getLayer(i) != nullptr) {
				this->getLayer(i)->render(deltaTime, context);
			}
		}

		#ifdef EGGINE_DEBUG
		this->isCulled = false;
		#endif
	}
	#ifdef EGGINE_DEBUG
	else {
		this->isCulled = true;
	}
	#endif
}

void Chunk::addOverlappingTile(OverlappingTile* tile) {
	Layer* found = this->getLayer(tile->getPosition().z);
	if(found == nullptr) {
		found = this->layers[tile->getPosition().z] = new Layer(this);
		this->maxLayer = max(tile->getPosition().z, this->maxLayer);
	}

	this->overlappingTiles.insert(tile);

	found->add(tile);
}

void Chunk::updateOverlappingTile(OverlappingTile* tile) {
	Layer* found = this->getLayer(tile->getPosition().z);
	if(found == nullptr) {
		found = this->layers[tile->getPosition().z] = new Layer(this);
		this->maxLayer = max(tile->getPosition().z, this->maxLayer);
	}
	
	if(found != tile->getLayer()) {
		tile->getLayer()->remove(tile);
		found->add(tile);
	}
	else {
		found->update(tile);
	}
}
	
void Chunk::removeOverlappingTile(OverlappingTile* tile) {
	if(tile->getLayer() != nullptr) {
		tile->getLayer()->remove(tile);
	}

	this->overlappingTiles.erase(tile);
}

Layer* Chunk::getLayer(unsigned int z) {
	auto found = this->layers.find(z);
	if(found == this->layers.end()) {
		return nullptr;
	}
	return found.value();
}

void Chunk::setTileTexture(glm::uvec3 position, unsigned int spritesheetIndex) {
	glm::uvec2 relativePosition = glm::uvec2(position.x, position.y) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size) + position.z * Chunk::Size * Chunk::Size;

	this->vertexBuffer->setSubData(&spritesheetIndex, 1, index * sizeof(unsigned int));
	this->textureIndices[index] = spritesheetIndex;

	this->height = max(this->height, position.z + 1);
}

int Chunk::getTileTexture(glm::uvec3 position) {
	glm::uvec2 relativePosition = glm::uvec2(position.x, position.y) - this->position * (unsigned int)Chunk::Size;
	unsigned int index = tilemath::coordinateToIndex(relativePosition, Chunk::Size) + position.z * Chunk::Size * Chunk::Size;

	if(index > Chunk::Size * Chunk::Size * Chunk::MaxHeight) {
		return 0;
	}
	return this->textureIndices[index];
}
