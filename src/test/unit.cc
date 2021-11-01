#include "unit.h"

#include "chunkContainer.h"

Unit::Unit(ChunkContainer* chunkContainer) : Character(chunkContainer) {
	this->reference = esInstantiateObject(engine->eggscript, "Unit", this);
}

Unit::~Unit() {
	esDeleteObject(this->reference);
}

void es::defineUnit() {
	esRegisterNamespace(engine->eggscript, "Unit");
	esNamespaceInherit(engine->eggscript, "Character", "Unit");

	esEntryType destinationArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, es::Unit__getDestinations, "Unit", "getDestinations", 1, destinationArguments);
}

esEntryPtr es::Unit__getDestinations(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Unit")) {
		esEntryPtr entry = new esEntry();
		entry->type = ES_ENTRY_OBJECT;
		entry->objectData = ((Unit*)args[0].objectData->objectWrapper->data)->destinations.reference;
		return entry;
	}
	return nullptr;
}
