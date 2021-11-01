#include "character.h"

#include "chunkContainer.h"

Character::Character(ChunkContainer* chunkContainer) : OverlappingTile(chunkContainer) {
	this->reference = esInstantiateObject(engine->eggscript, "Character", this);
}

Character::~Character() {
	esDeleteObject(this->reference);
}

void Character::setPosition(glm::uvec3 position) {
	this->container->updateCharacterPosition(this, position);
	OverlappingTile::setPosition(position);
}

bool Character::move(glm::uvec3 position) {
	if(this->moveTest(position)) {
		this->setPosition(position);
		return true;
	}
	else {
		return false;
	}
}

bool Character::moveTest(glm::uvec3 position) {
	esEntry arguments[3];
	arguments[0].type = ES_ENTRY_NUMBER;
	arguments[0].numberData = position.x;
	arguments[1].type = ES_ENTRY_NUMBER;
	arguments[1].numberData = position.y;
	arguments[2].type = ES_ENTRY_NUMBER;
	arguments[2].numberData = position.z;

	esEntryPtr result = esCallMethod(engine->eggscript, this->reference, "moveTest", 3, arguments);
	bool output = result->numberData == 1.0;
	delete result;
	return output;
}

void es::defineCharacter() {
	esRegisterNamespace(engine->eggscript, "Character");
	esNamespaceInherit(engine->eggscript, "SimObject", "Character");

	esEntryType setPositionArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::Character__setPosition, "Character", "setPosition", 2, setPositionArguments);

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
