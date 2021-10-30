#include "team.h"

#include "character.h"
#include "../engine/engine.h"

Team::Team() {
	// create torquescript object
	this->reference = esInstantiateObject(engine->eggscript, "Team", this);
}

void Team::add(Character* character) {
	this->characters.push_back(character);
}

void es::defineTeam() {
	esRegisterNamespace(engine->eggscript, "Team");
	esNamespaceInherit(engine->eggscript, "SimObject", "Team");

	esEntryType sizeArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Team__size, "Team", "size", 1, sizeArguments);

	esEntryType getArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Team__get, "Team", "get", 2, getArguments);
}

esEntryPtr es::Team__size(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		return esCreateNumber(team->characters.size());
	}
	return nullptr;
}

esEntryPtr es::Team__get(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Team")) {
		Team* team = (Team*)args[0].objectData->objectWrapper->data;
		int index = args[1].numberData;

		if(index >= team->characters.size() || index < 0) {
			return nullptr;
		}
		return esCreateObject(team->characters[index]->reference);
	}
	return nullptr;
}
