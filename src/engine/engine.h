#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glfw/glfw3.h>
#include <tsl/robin_map.h>
#include <torquescript/ts.h>
#include <vector>

#include "../basic/camera.h"
#include "debug.h"
#include "../util/dynamicArray.h"
#include "keybind.h"
#include "../basic/text.h"
#include "../basic/renderObject.h"
#include "../basic/ui.h"

using namespace std;
using namespace tsl;

void engineInitRenderables(class Engine*, RenderObject** object);

class Engine {
	friend Camera;
	friend void onKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);
	friend tsEntryPtr ts::getActiveCamera(tsEnginePtr tsEngine, unsigned int argc, tsEntry* args);
	
	public:
		void initialize();
		void tick();
		void exit();

		void addRenderObject(RenderObject* renderable);
		void addUIObject(RenderObject* renderable);

		// handle keybinds
		void registerBindPress(string command, GameObject* gameObject);
		void registerBindRelease(string command, GameObject* gameObject);
		void registerBindHeld(string command, GameObject* gameObject);
		void addKeybind(int key, binds::Keybind keybind);

		int windowWidth;
		int windowHeight;

		FT_Library ft;
		tsEnginePtr torquescript = nullptr;

		#ifdef EGGINE_DEBUG
		Debug debug;
		#endif

	private:
		long long cpuRenderTime = 0;
		long long torquescriptTickTime = 0;
		
		GLFWwindow* window;
		Camera* camera = nullptr;
		UI ui;
		long long lastRenderTime;

		#ifdef EGGINE_DEBUG
		Text* debugText;
		#endif

		DynamicArray<RenderObject*, Engine> renderables = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);
		DynamicArray<RenderObject*, Engine> renderableUIs = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);

		robin_map<string, vector<GameObject*>> bindPressToGameObject;
		robin_map<string, vector<GameObject*>> bindReleaseToGameObject;
		robin_map<string, vector<GameObject*>> bindHeldToGameObject;
		robin_map<int, vector<binds::Keybind>> keyToKeybind;
		vector<pair<GameObject*, string>> heldEvents;
};

extern Engine* engine;
