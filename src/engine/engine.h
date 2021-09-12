#pragma once

#include <glfw/glfw3.h>
#include <vector>

#include "../basic/camera.h"
#include "../util/dynamicArray.h"
#include "../basic/renderObject.h"

using namespace std;

void engineInitRenderables(class Engine*, RenderObject** object);

class Engine {
	public:
		void initialize();
		void tick();
		void exit();

		void addRenderObject(RenderObject* renderable);

		int windowWidth;
		int windowHeight;

	private:
		GLFWwindow* window;
		Camera camera;
		long long lastRenderTime;		

		DynamicArray<RenderObject*, Engine> renderables = DynamicArray<RenderObject*, Engine>(this, 1024, engineInitRenderables, nullptr);
};

extern Engine* engine;
