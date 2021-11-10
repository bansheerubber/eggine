#include "overlappingTile.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"

OverlappingTile::OverlappingTile(ChunkContainer* container, bool createReference) : GameObject() {
	this->container = container;

	if(createReference) {
		this->reference = esInstantiateObject(engine->eggscript, "OverlappingTile", this);
	}
}

OverlappingTile::~OverlappingTile() {
	if(this->chunk != nullptr) {
		this->chunk->removeOverlappingTile(this);
	}

	if(this->reference != nullptr) {
		this->reference = nullptr;
	}
}

OverlappingTile* OverlappingTile::setPosition(glm::uvec3 position) {
	if(!this->container->isValidTilePosition(position)) {
		return this;
	}
	
	this->position = position;

	glm::uvec2 chunkPosition(position.x / Chunk::Size, position.y / Chunk::Size);
	Chunk* chunk = this->container->getChunk(tilemath::coordinateToIndex(chunkPosition, this->container->getSize(), this->container->getRotation()));

	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= chunk->position.x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk->position.y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition, this->container->getRotation());

	bool initialized = false;
	if(this->chunk != chunk) {
		if(this->chunk != nullptr) {
			this->chunk->removeOverlappingTile(this);
		}
		chunk->addOverlappingTile(this);
		initialized = true;
	}
	this->chunk = chunk;

	if(!initialized) {
		this->chunk->updateOverlappingTile(this);
	}

	this->layer = this->chunk->getLayer(position.z);

	return this;
}

void OverlappingTile::updateRotation(tilemath::Rotation rotation) {
	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= chunk->position.x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk->position.y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition, this->container->getRotation());
}

glm::uvec3 OverlappingTile::getPosition() {
	return this->position;
}

Layer* OverlappingTile::getLayer() {
	return this->layer;
}

OverlappingTile* OverlappingTile::setTexture(unsigned int index) {
	this->textureIndex = index;
	if(this->chunk != nullptr) {
		this->chunk->updateOverlappingTile(this);
	}
	return this;
}

int OverlappingTile::getTexture() {
	return this->textureIndex;
}

OverlappingTile* OverlappingTile::setColor(glm::vec4 color) {
	this->color = color;
	if(this->chunk != nullptr) {
		this->chunk->updateOverlappingTile(this);
	}
	return this;
}

glm::vec4 OverlappingTile::getColor() {
	return this->color;
}

OverlappingTile* OverlappingTile::setZIndex(unsigned int zIndex) {
	this->zIndex = zIndex;
	return this;
}

unsigned int OverlappingTile::getZIndex() {
	return this->zIndex;	
}

ChunkContainer* OverlappingTile::getContainer() {
	return this->container;
}

void es::defineOverlappingTile() {
	esRegisterNamespace(engine->eggscript, "OverlappingTile");
	esNamespaceInherit(engine->eggscript, "SimObject", "OverlappingTile");
	esSetNamespaceConstructor(engine->eggscript, "OverlappingTile", es::OverlappingTile__constructor);
	esSetNamespaceDeconstructor(engine->eggscript, "OverlappingTile", es::OverlappingTile__deconstructor);

	esEntryType setPositionArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::OverlappingTile__setPosition, "OverlappingTile", "setPosition", 2, setPositionArguments);

	esEntryType getPositionArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::OverlappingTile__getPosition, "OverlappingTile", "getPosition", 1, getPositionArguments);

	esEntryType setTextureArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::OverlappingTile__setTexture, "OverlappingTile", "setTexture", 2, setTextureArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::OverlappingTile__getTexture, "OverlappingTile", "getTexture", 1, getPositionArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::OverlappingTile__setColor, "OverlappingTile", "setColor", 2, setPositionArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::OverlappingTile__getColor, "OverlappingTile", "getColor", 1, getPositionArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::OverlappingTile__setZIndex, "OverlappingTile", "setZIndex", 2, setTextureArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::OverlappingTile__getZIndex, "OverlappingTile", "getZIndex", 1, getPositionArguments);
}

void es::OverlappingTile__constructor(esObjectWrapperPtr wrapper) {
	wrapper->data = new OverlappingTile(engine->chunkContainer);
}

void es::OverlappingTile__deconstructor(esObjectWrapperPtr wrapper) {
	delete (OverlappingTile*)wrapper->data;
}

esEntryPtr es::OverlappingTile__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		((OverlappingTile*)args[0].objectData->objectWrapper->data)->setPosition(
			glm::uvec3(args[1].matrixData->data[0][0].numberData, args[1].matrixData->data[1][0].numberData, args[1].matrixData->data[2][0].numberData)
		);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		glm::uvec3 position = ((OverlappingTile*)args[0].objectData->objectWrapper->data)->getPosition();
		return esCreateVector(3, (double)position.x, (double)position.y, (double)position.z);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__setTexture(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		((OverlappingTile*)args[0].objectData->objectWrapper->data)->setTexture(args[1].numberData);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__getTexture(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		return esCreateNumber(((OverlappingTile*)args[0].objectData->objectWrapper->data)->getTexture());
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__setColor(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile") && args[1].matrixData->rows == 4 && args[1].matrixData->columns == 1) {
		((OverlappingTile*)args[0].objectData->objectWrapper->data)->setColor(
			glm::vec4(
				args[1].matrixData->data[0][0].numberData,
				args[1].matrixData->data[1][0].numberData,
				args[1].matrixData->data[2][0].numberData,
				args[1].matrixData->data[3][0].numberData
			)
		);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__getColor(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		glm::vec4 color = ((OverlappingTile*)args[0].objectData->objectWrapper->data)->getColor();
		return esCreateVector(4, (double)color.r, (double)color.g, (double)color.b, (double)color.a);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__setZIndex(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		((OverlappingTile*)args[0].objectData->objectWrapper->data)->setZIndex(args[1].numberData);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__getZIndex(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		return esCreateNumber(((OverlappingTile*)args[0].objectData->objectWrapper->data)->getZIndex());
	}
	return nullptr;
}
