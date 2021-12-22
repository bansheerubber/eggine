#pragma once

#include "developer.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <tsl/robin_map.h>
#include <eggscript/egg.h>
#include <set>
#include <vector>

#include "../basic/camera.h"
#include "callbacks.h"
#include "debug.h"
#include "../util/dynamicArray.h"
#include "../sound/engine.h"
#include "keybind.h"
#include "network.h"
#include "../basic/text.h"
#include "../basic/renderObject.h"
#include "../resources/resourceManager.h"
#include "../basic/ui.h"
#include "../renderer/window.h"

using namespace std;

void engineInitRenderables(class Engine*, RenderObject** object);

namespace std {
	template<>
	struct hash<pair<esObjectReferencePtr, string>> {
		size_t operator()(pair<esObjectReferencePtr, string> const& source) const noexcept {
			size_t result = hash<esObjectWrapperPtr>{}(source.first->objectWrapper);
			return result ^ (hash<string>{}(source.second) + 0x9e3779b9 + (result << 6) + (result >> 2));
    }
	};

	template<>
	struct equal_to<pair<esObjectReferencePtr, string>> {
		bool operator()(const pair<esObjectReferencePtr, string>& x, const pair<esObjectReferencePtr, string>& y) const {
			return x.first->objectWrapper == y.first->objectWrapper && x.second == y.second;
		}
	};
	
	template<>
	struct hash<esObjectReferencePtr> {
		size_t operator()(esObjectReferencePtr const& source) const noexcept {
			return hash<esObjectWrapperPtr>{}(source->objectWrapper);
    }
	};
	
	template<> // specialization
	inline bool equal<esObjectReferencePtr, esObjectReferencePtr>(esObjectReferencePtr first1, esObjectReferencePtr last1, esObjectReferencePtr first2) noexcept {
		for(; first1 != last1; ++first1, ++first2) {
			if(!(first1->objectWrapper == first2->objectWrapper)) {
				return false;
			}
		}
    return true;
	}
};

class Engine {
	friend Camera;
	friend class Shader;
	friend class Font;
	#ifndef __switch__
	friend void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);
	friend void onMouseMove(GLFWwindow* window, double x, double y);
	#endif
	friend esEntryPtr es::getActiveCamera(esEnginePtr esEngine, unsigned int argc, esEntry* args);
	friend esEntryPtr es::onKeyPress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	friend esEntryPtr es::onMousePress(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	friend esEntryPtr es::onAxisMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	friend esEntryPtr es::onGamepadButton(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	
	public:
		FT_Library ft;
		esEnginePtr eggscript = nullptr;
		resources::ResourceManager* manager;
		render::Window renderWindow;
		sound::Engine soundEngine;
		network::Network network;

		glm::vec2 mouse;

		Camera* camera = nullptr;
		class ChunkContainer* chunkContainer = nullptr;
		UI ui;

		#ifdef EGGINE_DEBUG
		Debug debug;
		#endif

		#ifdef EGGINE_DEVELOPER_MODE
		class DeveloperGui* developerGui;
		#endif
		
		void initialize();
		void tick();
		void exit();

		void addRenderObject(RenderObject* renderable);
		void removeRenderObject(RenderObject* renderable);

		void addUIObject(RenderObject* renderable);
		void removeUIObject(RenderObject* renderable);

		// handle keybinds
		void registerTSKeybindCallback(string bind, string key, string callback);
		void registerTSKeybindObjectCallback(esObjectReferencePtr object, string bind, string key, string callback);
		void registerBind(string command, GameObject* gameObject);
		void registerBindAxis(string command, GameObject* gameObject);
		void addKeybind(int key, binds::Keybind keybind);
		void addAxis(binds::Axes axis, binds::Keybind keybind);
		void addMousebind(int button, binds::Keybind keybind);
		void addGamepadBind(binds::GamepadButtons bind, binds::Keybind keybind);

		void setFilePrefix(string filePrefix);
		string getFilePrefix();

	private:
		#ifdef __switch__
		unsigned int nxlink = 0;
		#endif

		string filePrefix = "";
		
		long long cpuRenderTime = 0;
		long long eggscriptTickTime = 0;
		
		class Shader* boundShader = nullptr;
		long long lastRenderTime;

		#ifdef EGGINE_DEBUG
		Text* debugText;
		#endif

		DynamicArray<RenderObject*, Engine> renderables = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);
		DynamicArray<RenderObject*, Engine> renderableUIs = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);

		tsl::robin_map<string, tsl::robin_set<string>> bindToTSCallback;
		tsl::robin_map<string, tsl::robin_set<pair<esObjectReferencePtr, string>>> bindToTSObjectCallback;
		tsl::robin_map<string, tsl::robin_set<GameObject*>> bindToGameObject;
		tsl::robin_map<string, tsl::robin_set<GameObject*>> bindAxisToGameObject;

		tsl::robin_map<int, vector<binds::Keybind>> keyToKeybind;
		tsl::robin_map<binds::Axes, vector<binds::Keybind>> axisToKeybind;
		tsl::robin_map<int, vector<binds::Keybind>> buttonToMousebind;
		tsl::robin_map<binds::GamepadButtons, vector<binds::Keybind>> gamepadToBind;

		tsl::robin_map<string, int> keyToScancode;
		tsl::robin_map<int, string> scancodeToKey;

		tsl::robin_map<string, binds::GamepadButtons> gamepadToEnum;
		tsl::robin_map<string, binds::Axes> axesToEnum;
		tsl::robin_map<string, int> mouseToEnum;

		#ifdef __switch__
		unsigned long lastGamepadButtons = 0;
		#else
		unsigned char* lastGamepadButtons = new unsigned char[15]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		#endif
};

extern Engine* engine;
