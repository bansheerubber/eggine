#include "team.h"

#include "chunkContainer.h"
#include "../util/cloneString.h"
#include "../engine/engine.h"
#include "tileMath.h"
#include "unit.h"

std::vector<Team*> Team::Teams = std::vector<Team*>();

Team::Team() {
	// create torquescript object
	this->reference = esInstantiateObject(engine->eggscript, "Team", this);
	Team::Teams.push_back(this);
}

Team::~Team() {
	Team::Teams.erase(std::find(Team::Teams.begin(), Team::Teams.end(), this));
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
	if(*unit1 > *unit2) {
		return 1;
	}
	else if(*unit1 < *unit2) {
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

	esEntryType setNameArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_STRING};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, es::Team__setName, "Team", "setName", 2, setNameArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_STRING, es::Team__getName, "Team", "getName", 1, sizeArguments);

	esRegisterFunction(engine->eggscript, ES_ENTRY_NUMBER, es::getTeamCount, "getTeamCount", 0, nullptr);

	esEntryType getTeamArguments[1] = {ES_ENTRY_NUMBER};
	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, es::getTeam, "getTeam", 1, getTeamArguments);
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
		for(uint64_t i = 0; i < team->units.array.head; i++) {
			if(team->units.array[i] == unit) {
				return esCreateNumber(1);
			}
		}
	}
	return nullptr;
}

esEntryPtr es::Team__setName(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		team->name = std::string(args[1].stringData);
	}
	return nullptr;
}

esEntryPtr es::Team__getName(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		return esCreateString(cloneString(team->name.c_str()));
	}
	return nullptr;
}

esEntryPtr es::getTeamCount(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 0) {
		return esCreateNumber(Team::Teams.size());
	}
	return nullptr;
}

esEntryPtr es::getTeam(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		unsigned int index = (unsigned int)args[0].numberData;
		if((uint64_t)index >= Team::Teams.size()) {
			return nullptr;
		}
		return esCreateObject(Team::Teams[index]->reference);
	}
	return nullptr;
}
