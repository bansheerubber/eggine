#pragma once

#include <eggscript/egg.h>

namespace es {
	void defineDOM();
	esEntryPtr getHTMLElementById(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__getParent(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__addChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__removeChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__createChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__clear(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__setAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__getAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__setStyleAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
	esEntryPtr HTMLElement__getStyleAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args);
};
