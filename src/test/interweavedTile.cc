#include "interweavedTile.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"

InterweavedTile::InterweavedTile(bool createReference) : OverlappingTile(false) {
	this->vertexBuffers[0] = new render::VertexBuffer(&engine->renderWindow);
	this->vertexBuffers[1] = new render::VertexBuffer(&engine->renderWindow);
	this->vertexBuffers[2] = new render::VertexBuffer(&engine->renderWindow);
	this->vertexBuffers[3] = new render::VertexBuffer(&engine->renderWindow);

	this->vertexAttributes = new render::VertexAttributes(&engine->renderWindow);

	// load vertices
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// load uvs
	{
		this->vertexAttributes->addVertexAttribute(ChunkContainer::UVs, 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	// configure position buffer
	{
		this->vertexBuffers[0]->setDynamicDraw(true);
		this->vertexBuffers[0]->setData(&this->position, sizeof(this->position), sizeof(this->position));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffers[0], 2, 3, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec3), 1);
	}

	// configure texture buffer
	{
		this->vertexBuffers[1]->setDynamicDraw(true);
		this->vertexBuffers[1]->setData(&this->textureIndex, sizeof(this->textureIndex), sizeof(this->textureIndex));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffers[1], 3, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}

	// configure color buffer
	{
		this->vertexBuffers[2]->setDynamicDraw(true);
		this->vertexBuffers[2]->setData(&this->color, sizeof(this->color), sizeof(this->color));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffers[2], 4, 4, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 1);
	}

	// configure xray buffer
	{
		this->vertexBuffers[3]->setDynamicDraw(true);
		this->vertexBuffers[3]->setData(&this->xrayBuffer, sizeof(this->xrayBuffer), sizeof(this->xrayBuffer));
		this->vertexAttributes->addVertexAttribute(this->vertexBuffers[3], 5, 1, render::VERTEX_ATTRIB_INT, 0, sizeof(int), 1);
	}
}

InterweavedTile::~InterweavedTile() {
	if(this->chunk != nullptr) {
		this->chunk->removeInterweavedTile(this);
	}

	if(this->reference != nullptr) {
		this->reference = nullptr;
	}
}

OverlappingTile* InterweavedTile::setPosition(glm::uvec3 position) {
	if(!this->container->isValidTilePosition(position)) {
		return this;
	}
	
	this->position = position;

	glm::uvec2 chunkPosition(position.x / Chunk::Size, position.y / Chunk::Size);
	Chunk* chunk = this->container->getChunk(tilemath::coordinateToIndex(chunkPosition, this->container->getSize(), this->container->getRotation()));

	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= chunk->position.x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk->position.y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition, Chunk::Size, this->container->getRotation());
	
	bool drawOntop = ChunkContainer::Image->drawOntopOfOverlap(this->container->getTile(this->position));
	this->screenSpacePosition.z += (drawOntop ? -0.5 : 0.5) + (double)this->getZIndex() / 1000.0f;

	bool initialized = false;
	if(this->chunk != chunk) {
		if(this->chunk != nullptr) {
			this->chunk->removeInterweavedTile(this);
		}
		chunk->addInterweavedTile(this);
		initialized = true;
	}
	this->chunk = chunk;

	if(!initialized) {
		this->chunk->updateInterweavedTile(this);
	}

	this->layer = this->chunk->getLayer(position.z);

	this->vertexBuffers[0]->setData(&this->screenSpacePosition, sizeof(this->screenSpacePosition), sizeof(this->screenSpacePosition));

	return this;
}

void InterweavedTile::updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation) {
	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= chunk->position.x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk->position.y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition, Chunk::Size, this->container->getRotation());

	bool drawOntop = ChunkContainer::Image->drawOntopOfOverlap(this->container->getTile(this->position));
	this->screenSpacePosition.z += (drawOntop ? -0.5 : 0.5) + (double)this->getZIndex() / 1000.0f;

	this->vertexBuffers[0]->setData(&this->screenSpacePosition, sizeof(this->screenSpacePosition), sizeof(this->screenSpacePosition));

	resources::SpriteFacingInfo* facingsMap;
	if((facingsMap = ChunkContainer::Image->getSpriteInfo(this->textureIndex).facingsMap) != nullptr) {
		this->textureIndex = facingsMap->rotateFacing(ChunkContainer::Image->getSpriteInfo(this->textureIndex).facing, oldRotation, newRotation);
		this->vertexBuffers[1]->setData(&this->textureIndex, sizeof(this->textureIndex), sizeof(this->textureIndex));
	}
}

OverlappingTile* InterweavedTile::setTexture(unsigned int index) {
	this->textureIndex = index;
	if(this->chunk != nullptr) {
		this->chunk->updateInterweavedTile(this);
	}

	this->vertexBuffers[1]->setData(&this->textureIndex, sizeof(this->textureIndex), sizeof(this->textureIndex));

	return this;
}

OverlappingTile* InterweavedTile::setColor(glm::vec4 color) {
	this->color = color;
	if(this->chunk != nullptr) {
		this->chunk->updateInterweavedTile(this);
	}

	this->vertexBuffers[2]->setData(&this->color, sizeof(this->color), sizeof(this->color));

	return this;
}

OverlappingTile* InterweavedTile::setZIndex(unsigned int zIndex) {
	this->zIndex = zIndex;
	if(this->chunk != nullptr) {
		this->chunk->updateInterweavedTile(this);
	}
	return this;
}

void InterweavedTile::render(double deltaTime, RenderContext &context) {
	this->xrayBuffer = 0;
	this->vertexBuffers[3]->setData(&this->xrayBuffer, sizeof(this->xrayBuffer), sizeof(this->xrayBuffer));
	
	this->vertexAttributes->bind();
	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, 1);
}

void InterweavedTile::renderXRay(double deltaTime, RenderContext &context) {
	this->xrayBuffer = this->canXRay();
	this->vertexBuffers[3]->setData(&this->xrayBuffer, sizeof(this->xrayBuffer), sizeof(this->xrayBuffer));

	engine->renderWindow.enableDepthTest(false);
	this->vertexAttributes->bind();
	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, 1);
	engine->renderWindow.enableDepthTest(true);
}

void es::defineInterweavedTile() {
	esRegisterNamespace(engine->eggscript, "InterweavedTile");
	esNamespaceInherit(engine->eggscript, "OverlappingTile", "InterweavedTile");
	esSetNamespaceConstructor(engine->eggscript, "InterweavedTile", es::InterweavedTile__constructor);
	esSetNamespaceDeconstructor(engine->eggscript, "InterweavedTile", es::InterweavedTile__deconstructor);
}

void es::InterweavedTile__constructor(esObjectWrapperPtr wrapper) {
	wrapper->data = new InterweavedTile(engine->chunkContainer);
}

void es::InterweavedTile__deconstructor(esObjectWrapperPtr wrapper) {
	delete (InterweavedTile*)wrapper->data;
}
