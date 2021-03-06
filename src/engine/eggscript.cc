#include "eggscript.h"

#include <filesystem>

#include "console.h"
#include "engine.h"
#include "../resources/scriptFile.h"
#include "../util/time.h"

void es::eggscriptDefinitions() {
	esEntryType addKeybindArguments[3] = {ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, &addKeybind, "addKeybind", 3, addKeybindArguments);

	esEntryType addKeybindToObjectArguments[4] = {ES_ENTRY_OBJECT, ES_ENTRY_STRING, ES_ENTRY_STRING, ES_ENTRY_STRING};
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, &SimObject__addKeybind, "SimObject", "addKeybind", 4, addKeybindToObjectArguments);

	esEntryType execArguments[1] = {ES_ENTRY_STRING};
	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, &exec, "exec", 1, execArguments);

	esRegisterFunction(engine->eggscript, ES_ENTRY_NUMBER, &probeGarbage, "probeGarbage", 1, execArguments);

	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, &startBenchmark, "startBenchmark", 0, nullptr);
	esRegisterFunction(engine->eggscript, ES_ENTRY_EMPTY, &endBenchmark, "endBenchmark", 0, nullptr);
}

esEntryPtr es::exec(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		std::string path = std::filesystem::path(esGetLastExecFileName(esEngine)).parent_path().string();
		std::replace(path.begin(), path.end(), '\\', '/');
		if(path != "") {
			path += "/";
		}
		path += args[0].stringData;

		resources::ScriptFile* file = (resources::ScriptFile*)engine->manager.metadataToResources(
			engine->manager.carton->database.get()->equals("fileName", path)->exec()
		)[0];

		if(file != nullptr) {
			esExecVirtualFile(esEngine, path.c_str(), file->script.c_str());	
			console::print("executed '%s'\n", path.c_str());
		}
		else {
			console::error("could not find file '%s' to execute\n", path.c_str());
		}
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

esEntryPtr es::probeGarbage(esEnginePtr engine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		return esCreateNumber(esProbeGarbage(engine, args[0].stringData));
	}
	return nullptr;
}

uint64_t startTime;

esEntryPtr es::startBenchmark(esEnginePtr engine, unsigned int argc, esEntryPtr args) {
	startTime = getMicrosecondsNow();
	return nullptr;
}

esEntryPtr es::endBenchmark(esEnginePtr engine, unsigned int argc, esEntryPtr args) {
	console::print("%lld\n", getMicrosecondsNow() - startTime);
	return nullptr;
}
