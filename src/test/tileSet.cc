#include "tileSet.h"

#include "../engine/engine.h"

TileSet::TileSet() {
	this->reference = esInstantiateObject(engine->eggscript, "TileSet", this);
}

TileSet::~TileSet() {
	esDeleteObject(this->reference);
}

bool TileSet::add(glm::uvec2 position) {
	bool alreadyHas = this->has(position);
	if(!alreadyHas) {
		this->set.insert(this->index(position));
	}
	return alreadyHas;
}

bool TileSet::remove(glm::uvec2 position) {
	bool alreadyHas = this->has(position);
	if(alreadyHas) {
		this->set.erase(this->index(position));
	}
	return alreadyHas;
}

bool TileSet::has(glm::uvec2 position) {
	return this->set.find(this->index(position)) != this->set.end();
}

void TileSet::clear() {
	this->set.clear();
}

unsigned int TileSet::index(glm::uvec2 &position) {
	return ((position.x + position.y) * (position.x + position.y + 1)) / 2 + position.y;
}

void es::defineTileSet() {
	esRegisterNamespace(engine->eggscript, "TileSet");
	esNamespaceInherit(engine->eggscript, "SimObject", "TileSet");

	esEntryType addArguments[3] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__add, "TileSet", "add", 3, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__remove, "TileSet", "remove", 3, addArguments);
	esRegisterMethod(engine->eggscript, ES_ENTRY_NUMBER, es::TileSet__has, "TileSet", "has", 3, addArguments);

	esEntryType clearArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::TileSet__clear, "TileSet", "clear", 1, clearArguments);
}

esEntryPtr es::TileSet__add(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 3 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->add(glm::uvec2((int)args[1].numberData, (int)args[2].numberData));
		return entry;
	}

	return nullptr;
}

esEntryPtr es::TileSet__remove(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 3 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->remove(glm::uvec2((int)args[1].numberData, (int)args[2].numberData));
		return entry;
	}

	return nullptr;
}

esEntryPtr es::TileSet__has(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 3 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_NUMBER;
		entry->numberData = ((TileSet*)args[0].objectData->objectWrapper->data)->has(glm::uvec2((int)args[1].numberData, (int)args[2].numberData));
		return entry;
	}

	return nullptr;
}

esEntryPtr es::TileSet__clear(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "TileSet")) {
		((TileSet*)args[0].objectData->objectWrapper->data)->clear();
	}

	return nullptr;
}
