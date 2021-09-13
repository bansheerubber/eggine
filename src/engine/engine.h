#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glfw/glfw3.h>
#include <vector>

#include "../basic/camera.h"
#include "../util/dynamicArray.h"
#include "../basic/text.h"
#include "../basic/renderObject.h"
#include "../basic/ui.h"

using namespace std;

void engineInitRenderables(class Engine*, RenderObject** object);

class Engine {
	friend Camera;
	
	public:
		void initialize();
		void tick();
		void exit();

		void addRenderObject(RenderObject* renderable);
		void addUIObject(RenderObject* renderable);

		int windowWidth;
		int windowHeight;

		FT_Library ft;

	private:
		long long cpuRenderTime = 0.0f;
		
		GLFWwindow* window;
		Camera camera;
		UI ui;
		long long lastRenderTime;

		Text* debugText;

		DynamicArray<RenderObject*, Engine> renderables = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);
		DynamicArray<RenderObject*, Engine> renderableUIs = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);
};

extern Engine* engine;
