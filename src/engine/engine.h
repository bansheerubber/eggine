#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <tsl/robin_map.h>
#include <eggscript/egg.h>
#include <vector>

#include "../basic/camera.h"
#include "callbacks.h"
#include "debug.h"
#include "../util/dynamicArray.h"
#include "keybind.h"
#include "../basic/text.h"
#include "../basic/renderObject.h"
#include "../resources/resourceManager.h"
#include "../basic/ui.h"
#include "../renderer/window.h"

using namespace std;

void engineInitRenderables(class Engine*, RenderObject** object);

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
	friend esEntryPtr es::onMouseMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	friend esEntryPtr es::onAxisMove(esEnginePtr esEngine, unsigned int argc, esEntryPtr arguments);
	
	public:
		FT_Library ft;
		esEnginePtr eggscript = nullptr;
		resources::ResourceManager* manager;
		render::Window renderWindow;

		glm::vec2 mouse;

		Camera* camera = nullptr;

		#ifdef EGGINE_DEBUG
		Debug debug;
		#endif
		
		void initialize();
		void tick();
		void exit();

		void addRenderObject(RenderObject* renderable);
		void addUIObject(RenderObject* renderable);

		// handle keybinds
		void registerBindPress(string command, GameObject* gameObject);
		void registerBindRelease(string command, GameObject* gameObject);
		void registerBindHeld(string command, GameObject* gameObject);
		void registerBindAxis(string command, GameObject* gameObject);
		void addKeybind(int key, binds::Keybind keybind);
		void addAxis(int axis, binds::Keybind keybind);
		void addMousebind(int button, binds::Keybind keybind);
		void registerTSKeybindCallback(string bind, string key, string callback);

		void setFilePrefix(string filePrefix);

	private:
		#ifdef __switch__
		unsigned int nxlink = 0;
		#endif

		string filePrefix = "";
		
		long long cpuRenderTime = 0;
		long long eggscriptTickTime = 0;
		
		class Shader* boundShader = nullptr;
		UI ui;
		long long lastRenderTime;

		#ifdef EGGINE_DEBUG
		Text* debugText;
		#endif

		DynamicArray<RenderObject*, Engine> renderables = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);
		DynamicArray<RenderObject*, Engine> renderableUIs = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);

		tsl::robin_map<string, vector<string>> bindToTSCallback;
		tsl::robin_map<string, vector<GameObject*>> bindPressToGameObject;
		tsl::robin_map<string, vector<GameObject*>> bindReleaseToGameObject;
		tsl::robin_map<string, vector<GameObject*>> bindHeldToGameObject;
		tsl::robin_map<string, vector<GameObject*>> bindAxisToGameObject;
		tsl::robin_map<int, vector<binds::Keybind>> keyToKeybind;
		tsl::robin_map<int, vector<binds::Keybind>> axisToKeybind;
		tsl::robin_map<int, vector<binds::Keybind>> buttonToMousebind;
		vector<pair<GameObject*, string>> heldEvents;

		tsl::robin_map<string, int> keyToScancode;
		tsl::robin_map<int, string> scancodeToKey;
};

extern Engine* engine;
