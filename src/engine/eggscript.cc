#include "eggscript.h"

#include <filesystem>

#include "engine.h"
#include "../resources/scriptFile.h"

void es::eggscriptDefinitions() {
	esEntryType addKeybindArguments[3] = {ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &addKeybind, "addKeybind", 3, addKeybindArguments);

	esEntryType addKeybindToObjectArguments[4] = {ES_ENTRY_OBJECT, ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, &SimObject__addKeybind, "SimObject", "addKeybind", 4, addKeybindToObjectArguments);

	esEntryType execArguments[1] = {ES_ENTRY_STRING};
	esRegisterFunction(engine->eggscript, ES_ENTRY_INVALID, &exec, "exec", 1, execArguments);
}

esEntryPtr es::exec(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		string path = filesystem::path(esGetLastExecFileName(esEngine)).parent_path();
		path += "/";
		path += args[0].stringData;

		resources::ScriptFile* file = (resources::ScriptFile*)engine->manager->metadataToResources(
			engine->manager->carton->database.get()->equals("fileName", path)->exec()
		)[0];
		esExecFileFromContents(esEngine, path.c_str(), file->script.c_str());
	}
	return nullptr;
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
