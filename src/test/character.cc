#include "character.h"

Character::Character() {
	this->reference = tsCreateObject(engine->torquescript, "Character", this);
}

Character::~Character() {
	tsDeleteObject(this->reference);
}

bool Character::move(glm::uvec2 position) {
	if(this->moveTest(position)) {
		this->position = position;
		return true;
	}
	else {
		return false;
	}
}

bool Character::moveTest(glm::uvec2 position) {
	tsEntry arguments[2];
	arguments[0].type = TS_ENTRY_NUMBER;
	arguments[0].numberData = position.x;
	arguments[1].type = TS_ENTRY_NUMBER;
	arguments[1].numberData = position.y;

	tsEntryPtr result = tsCallMethod(engine->torquescript, this->reference, "moveTest", 2, arguments);
	bool output = result->numberData == 1.0;
	delete result;
	return output;
}

void ts::defineCharacter() {
	tsRegisterNamespace(engine->torquescript, "Character");
	tsNamespaceInherit(engine->torquescript, "SimObject", "Character");

	tsEntryType destinationArguments[1] = {TS_ENTRY_OBJECT};
	tsRegisterMethod(engine->torquescript, TS_ENTRY_OBJECT, ts::Character__getDestinations, "Character", "getDestinations", 1, destinationArguments);
}

tsEntryPtr ts::Character__getDestinations(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args) {
	if(argc == 1 && tsCompareNamespaceToObject(args[0].objectData, "Character")) {
		tsEntryPtr entry = new tsEntry();
		entry->type = TS_ENTRY_OBJECT;
		entry->objectData = ((Character*)args[0].objectData->objectWrapper->data)->destinations.reference;
		return entry;
	}
	return nullptr;
}
