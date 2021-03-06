#include "overlappingTile.h"

#include "chunk.h"
#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"

OverlappingTile::OverlappingTile(bool createReference) : GameObject() {
	this->container = engine->chunkContainer;

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
	relativePosition.x -= chunk->getPosition().x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk->getPosition().y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition, Chunk::Size, this->container->getRotation());
	this->screenSpacePosition.z += 0.5;

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

void OverlappingTile::updateRotation(tilemath::Rotation oldRotation, tilemath::Rotation newRotation) {
	glm::uvec3 relativePosition = this->position;
	relativePosition.x -= chunk->getPosition().x * Chunk::Size; // we add the chunk position to the tile in the shader
	relativePosition.y -= chunk->getPosition().y * Chunk::Size;
	this->screenSpacePosition = tilemath::tileToScreen(relativePosition, Chunk::Size, newRotation);
	this->screenSpacePosition.z += 0.5;

	resources::SpriteFacingInfo* facingsMap;
	if((facingsMap = ChunkContainer::Image->getSpriteInfo(this->textureIndex).facingsMap) != nullptr) {
		this->textureIndex = facingsMap->rotateFacing(ChunkContainer::Image->getSpriteInfo(this->textureIndex).facing, oldRotation, newRotation);
	}
}

glm::uvec3 OverlappingTile::getPosition() const {
	return this->position;
}

Layer* OverlappingTile::getLayer() {
	return this->layer;
}

OverlappingTile* OverlappingTile::setXRay(int xray) {
	this->xray = xray;
	return this;
}

int OverlappingTile::canXRay() {
	return this->xray;
}

bool OverlappingTile::operator<(const OverlappingTile &other) const {
	unsigned int indexA = tilemath::coordinateToIndex(this->getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	unsigned int indexB = tilemath::coordinateToIndex(other.getPosition(), Chunk::Size, engine->chunkContainer->getRotation());

	if(indexA < indexB) {
		return true;
	}
	else if(indexA == indexB && this->getZIndex() < other.getZIndex()) {
		return true;
	}
	return false;
}

bool OverlappingTile::operator>(const OverlappingTile &other) const {
	unsigned int indexA = tilemath::coordinateToIndex(this->getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	unsigned int indexB = tilemath::coordinateToIndex(other.getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	
	if(indexA > indexB) {
		return true;
	}
	else if(indexA == indexB && this->getZIndex() > other.getZIndex()) {
		return true;
	}
	return false;
}

bool OverlappingTile::operator==(const OverlappingTile &other) const {
	unsigned int indexA = tilemath::coordinateToIndex(this->getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	unsigned int indexB = tilemath::coordinateToIndex(other.getPosition(), Chunk::Size, engine->chunkContainer->getRotation());
	if(indexA == indexB && this->getZIndex() == other.getZIndex()) {
		return true;
	}
	return false;
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

unsigned int OverlappingTile::getZIndex() const {
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
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::OverlappingTile__setPosition, "OverlappingTile", "setPosition", 2, setPositionArguments);

	esEntryType getPositionArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::OverlappingTile__getPosition, "OverlappingTile", "getPosition", 1, getPositionArguments);

	esEntryType setTextureArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::OverlappingTile__setTexture, "OverlappingTile", "setTexture", 2, setTextureArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::OverlappingTile__getTexture, "OverlappingTile", "getTexture", 1, getPositionArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::OverlappingTile__setColor, "OverlappingTile", "setColor", 2, setPositionArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::OverlappingTile__getColor, "OverlappingTile", "getColor", 1, getPositionArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::OverlappingTile__setZIndex, "OverlappingTile", "setZIndex", 2, setTextureArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::OverlappingTile__getZIndex, "OverlappingTile", "getZIndex", 1, getPositionArguments);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::OverlappingTile__setXRay, "OverlappingTile", "setXRay", 2, setTextureArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::OverlappingTile__getXRay, "OverlappingTile", "getXRay", 1, getPositionArguments);
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

esEntryPtr es::OverlappingTile__setXRay(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		((OverlappingTile*)args[0].objectData->objectWrapper->data)->setXRay(args[1].numberData);
	}
	return nullptr;
}

esEntryPtr es::OverlappingTile__getXRay(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObjectParents(args[0].objectData, "OverlappingTile")) {
		return esCreateNumber(((OverlappingTile*)args[0].objectData->objectWrapper->data)->canXRay());
	}
	return nullptr;
}
