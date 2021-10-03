#include "character.h"

Character::Character(ChunkContainer* chunkContainer) : OverlappingTile(chunkContainer) {
	this->reference = esCreateObject(engine->eggscript, "Character", this);
}

Character::~Character() {
	esDeleteObject(this->reference);
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
	esEntry arguments[2];
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

	esEntryType destinationArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Character__getDestinations, "Character", "getDestinations", 1, destinationArguments);
}

esEntryPtr es::Character__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Character")) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_OBJECT;
		entry->objectData = ((Character*)args[0].objectData->objectWrapper->data)->destinations.reference;
		return entry;
	}
	return nullptr;
}
