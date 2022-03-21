#include "dom.h"

#include "../engine/engine.h"
#include "../util/cloneString.h"

void es::defineDOM() {
	esRegisterNamespace(engine->eggscript, "HTMLElement");
	esNamespaceInherit(engine->eggscript, "SimObject", "HTMLElement");

	esEntryType getHTMLElementByIdArgs[1] = { ES_ENTRY_STRING };
	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, getHTMLElementById, "getHTMLElementById", 1, getHTMLElementByIdArgs);

	esEntryType getParentArgs[1] = { ES_ENTRY_OBJECT };
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, HTMLElement__getParent, "HTMLElement", "getParent", 1, getParentArgs);

	esEntryType addChildArgs[2] = { ES_ENTRY_OBJECT, ES_ENTRY_OBJECT };
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, HTMLElement__addChild, "HTMLElement", "addChild", 2, addChildArgs);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, HTMLElement__removeChild, "HTMLElement", "removeChild", 2, addChildArgs);

	esEntryType createChildArgs[2] = { ES_ENTRY_OBJECT, ES_ENTRY_STRING };
	esRegisterMethod(engine->eggscript, ES_ENTRY_OBJECT, HTMLElement__createChild, "HTMLElement", "createChild", 2, createChildArgs);

	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, HTMLElement__clear, "HTMLElement", "clear", 1, getParentArgs);

	esEntryType setAttributeArgs[3] = { ES_ENTRY_OBJECT, ES_ENTRY_STRING, ES_ENTRY_STRING };
	esRegisterMethod(engine->eggscript, ES_ENTRY_EMPTY, HTMLElement__setAttribute, "HTMLElement", "setAttribute", 3, setAttributeArgs);
	esRegisterMethod(engine->eggscript, ES_ENTRY_STRING, HTMLElement__getAttribute, "HTMLElement", "getAttribute", 2, createChildArgs);
	esRegisterMethod(engine->eggscript, ES_ENTRY_STRING, HTMLElement__setStyleAttribute, "HTMLElement", "setStyleAttribute", 3, setAttributeArgs);
	esRegisterMethod(engine->eggscript, ES_ENTRY_STRING, HTMLElement__getStyleAttribute, "HTMLElement", "getStyleAttribute", 2, createChildArgs);
}

esEntryPtr es::getHTMLElementById(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		esObjectReferencePtr object = engine->renderWindow.htmlContainer->getESObject(string(args[0].stringData->string, args[0].stringData->size));
		if(object != nullptr) {
			return esCreateObject(object);
		}
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__getParent(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		litehtml::element::ptr child = found.value();
		litehtml::element::ptr parent = child->parent();

		auto found2 = engine->renderWindow.htmlContainer->elementToESObject.find(&*parent);
		if(found2 == engine->renderWindow.htmlContainer->elementToESObject.end()) {
			return nullptr;
		}
		return esCreateObject(found2.value());
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__addChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		litehtml::element::ptr owner = found.value();

		found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[1].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		litehtml::element::ptr child = found.value();
		owner->appendChild(child);
		engine->renderWindow.registerHTMLUpdate();
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__removeChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		litehtml::element::ptr owner = found.value();

		found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[1].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		litehtml::element::ptr child = found.value();
		owner->removeChild(child);
		engine->renderWindow.registerHTMLUpdate();
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__createChild(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		litehtml::element::ptr parent = found.value();
		esEntryPtr entry = new esEntry;
		entry->type = ES_ENTRY_OBJECT;
		entry->objectData = engine->renderWindow.htmlContainer->createChild(parent, string(args[1].stringData->string, args[1].stringData->size));
		return entry;
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__clear(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 1) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}
		found.value()->clearRecursive();
		engine->renderWindow.registerHTMLUpdate();
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__setAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 3) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		std::string name(args[1].stringData->string, args[1].stringData->size);
		std::string value(args[2].stringData->string, args[2].stringData->size);

		found.value()->set_attr(name.c_str(), value.c_str());
		found.value()->parse_attributes();
		found.value()->parse_styles(true);
		engine->renderWindow.registerHTMLUpdate();

		esEntry arguments[3];
		esCreateObjectAt(&arguments[0], args[0].objectData);

		esStringPtr nameString = new esString {
			cloneString(name.c_str()),
			(uint16_t)name.size(),
		};
		esCreateStringAt(&arguments[1], nameString);

		esStringPtr valueString = new esString {
			cloneString(value.c_str()),
			(uint16_t)value.size(),
		};
		esCreateStringAt(&arguments[2], valueString);
		esDeleteEntry(esCallMethod(esEngine, args[0].objectData, "onSetAttribute", 3, arguments));
	}
	return nullptr;
}	

esEntryPtr es::HTMLElement__getAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		std::string name(args[1].stringData->string, args[1].stringData->size);
		const char* value = found.value()->get_attr(name.c_str());
		if(value != nullptr) {
			esStringPtr valueString = new esString {
				cloneString(value),
				(uint16_t)strlen(value),
			};
			return esCreateString(valueString);
		}
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__setStyleAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 3) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}
		litehtml::style styles;

		std::string name(args[1].stringData->string, args[1].stringData->size);
		std::string value(args[2].stringData->string, args[2].stringData->size);

		styles.add_property(name.c_str(), value.c_str(), nullptr, true);
		found.value()->add_style(styles);
		found.value()->parse_styles(true);
		engine->renderWindow.registerHTMLUpdate();
	}
	return nullptr;
}

esEntryPtr es::HTMLElement__getStyleAttribute(esEnginePtr esEngine, unsigned int argc, esEntryPtr args) {
	if(argc == 2) {
		auto found = engine->renderWindow.htmlContainer->esObjectToElement.find(args[0].objectData->objectWrapper);
		if(found == engine->renderWindow.htmlContainer->esObjectToElement.end()) {
			return nullptr;
		}

		std::string name(args[1].stringData->string, args[1].stringData->size);
		const char* value = found.value()->get_style_property(name.c_str(), false, nullptr);
		if(value != nullptr) {
			esStringPtr valueString = new esString {
				cloneString(value),
				(uint16_t)strlen(value),
			};
			return esCreateString(valueString);
		}
	}
	return nullptr;
}
