#include "character.h"

#include "chunkContainer.h"

Character::Character(bool createReference) : InterweavedTile(false) {
	if(createReference) {
		this->reference = esInstantiateObject(engine->eggscript, "Character", this);
	}
}

Character::~Character() {
	esDeleteObject(this->reference);
	this->container->positionToCharacter.erase(this->getPosition());
}

OverlappingTile* Character::setPosition(glm::uvec3 position) {
	if(
		this->unpacking
		&& (
			position.x >= engine->chunkContainer->getSize() * Chunk::Size
			|| position.y >= engine->chunkContainer->getSize() * Chunk::Size
		)
	) {
		throw network::RemoteObjectUnpackException(this, "Character: invalid position");
	}
	
	this->container->updateCharacterPosition(this, position);
	this->position = position;
	this->writeUpdateMask("position");

	return InterweavedTile::setPosition(position);
}

// texture should be north texture for the unit
void Character::setAppearance(unsigned int texture) {
	// TODO bounds check for network
	
	this->appearance = texture;
	this->setTexture(texture);
	this->writeUpdateMask("appearance");
}

void Character::setDirection(resources::SpriteFacing direction) {
	this->direction = direction;

	resources::SpriteFacingInfo* facingsMap;
	if((facingsMap = ChunkContainer::Image->getSpriteInfo(this->appearance).facingsMap) != nullptr) {
		this->setTexture(facingsMap->getFacing(direction, engine->chunkContainer->rotation));
	}
}

void es::defineCharacter() {
	esRegisterNamespace(engine->eggscript, "Character");
	esNamespaceInherit(engine->eggscript, "OverlappingTile", "Character");

	esEntryType setPositionArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Character__setPosition, "Character", "setPosition", 2, setPositionArguments);

	esEntryType getPositionArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::Character__getPosition, "Character", "getPosition", 1, getPositionArguments);

	esEntryType setDirectionArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Character__setDirection, "Character", "setDirection", 2, setDirectionArguments);
}

esEntryPtr es::Character__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "Character") && args[1].matrixData->rows == 3 && args[1].matrixData->columns == 1) {
		((Character*)args[0].objectData->objectWrapper->data)->setPosition(
			glm::uvec3(args[1].matrixData->data[0][0].numberData, args[1].matrixData->data[1][0].numberData, args[1].matrixData->data[2][0].numberData)
		);
	}
	return nullptr;
}

esEntryPtr es::Character__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObjectParents(args[0].objectData, "Character")) {
		glm::uvec3 position = ((Character*)args[0].objectData->objectWrapper->data)->getPosition();
		return esCreateVector(3, (double)position.x, (double)position.y, (double)position.z);
	}
	return nullptr;
}

esEntryPtr es::Character__setDirection(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObjectParents(args[0].objectData, "Character")) {
		unsigned int direction = args[1].numberData;
		if(direction < resources::FACING_NORTH || direction > resources::FACING_NORTH_WEST) {
			return nullptr;
		}
		
		((Character*)args[0].objectData->objectWrapper->data)->setDirection((resources::SpriteFacing)direction);
	}
	return nullptr;
}

