#include "character.h"

#include "chunkContainer.h"

Character::Character(ChunkContainer* chunkContainer, bool createReference) : InterweavedTile(chunkContainer, false) {
	if(createReference) {
		this->reference = esInstantiateObject(engine->eggscript, "Character", this);
	}
}

Character::~Character() {
	esDeleteObject(this->reference);
}

OverlappingTile* Character::setPosition(glm::uvec3 position) {
	this->container->updateCharacterPosition(this, position);
	return InterweavedTile::setPosition(position);
}

void es::defineCharacter() {
	esRegisterNamespace(engine->eggscript, "Character");
	esNamespaceInherit(engine->eggscript, "OverlappingTile", "Character");

	esEntryType setPositionArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Character__setPosition, "Character", "setPosition", 2, setPositionArguments);

	esEntryType getPositionArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::Character__getPosition, "Character", "getPosition", 1, getPositionArguments);
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
