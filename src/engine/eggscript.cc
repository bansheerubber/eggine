#include "eggscript.h"

#include "engine.h"

void es::eggscriptDefinitions() {
	esEntryType addKeybindArguments[3] = {ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &addKeybind, "addKeybind", 3, addKeybindArguments);

	esEntryType addKeybindToObjectArguments[4] = {ES_ENTRY_OBJECT, ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, &SimObject__addKeybind, "SimObject", "addKeybind", 4, addKeybindToObjectArguments);
}

esEntryPtr es::addKeybind(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 3) {
		engine->registerTSKeybindCallback(args[0].stringData, args[1].stringData, args[2].stringData);
	}
	return nullptr;
}

esEntryPtr es::SimObject__addKeybind(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 4) {
		engine->registerTSKeybindObjectCallback(args[0].objectData, args[1].stringData, args[2].stringData, args[3].stringData);
	}
	return nullptr;
}
