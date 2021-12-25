#include "team.h"

#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"
#include "unit.h"

Team::Team() {
	// create torquescript object
	this->reference = esInstantiateObject(engine->eggscript, "Team", this);
}

void Team::add(Unit* unit) {
	this->units.insert(unit);
	this->units.sort();
	unit->team = this;
}

void Team::remove(Unit* unit) {
	this->units.remove(unit);
	this->units.sort();
	unit->team = nullptr;
}

int teamUnitSort(Unit** unit1, Unit** unit2) {
	glm::vec2 screen1 = tilemath::tileToScreen((*unit1)->getPosition(), engine->chunkContainer->getRotation());
	glm::vec2 screen2 = tilemath::tileToScreen((*unit2)->getPosition(), engine->chunkContainer->getRotation());
	
	if(screen1.x > screen2.x) {
		return 1;
	}
	else if(screen1.x < screen2.x) {
		return -1;
	}
	else {
		return 0;
	}
}

void es::defineTeam() {
	esRegisterNamespace(engine->eggscript, "Team");
	esNamespaceInherit(engine->eggscript, "SimObject", "Team");

	esEntryType sizeArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Team__size, "Team", "size", 1, sizeArguments);

	esEntryType getArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Team__get, "Team", "get", 2, getArguments);

	esEntryType hasArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Team__has, "Team", "has", 2, hasArguments);
}

esEntryPtr es::Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		return esCreateNumber(team->units.array.head);
	}
	return nullptr;
}

esEntryPtr es::Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		int index = args[1].numberData;
		team->units.sort();

		if(index < 0 || (unsigned int)index >= team->units.array.head) {
			return nullptr;
		}
		return esCreateObject(team->units.array[index]->reference);
	}
	return nullptr;
}

esEntryPtr es::Team__has(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		Unit* unit = (Unit*)args[1].objectData->objectWrapper->data;
		for(size_t i = 0; i < team->units.array.head; i++) {
			if(team->units.array[i] == unit) {
				return esCreateNumber(1);
			}
		}
	}
	return nullptr;
}
