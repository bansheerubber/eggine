#pragma once

#include <glfw/glfw3.h>
#include <vector>

#include "../basic/renderObject.h"

using namespace std;

class Engine {
	public:
		void initialize();
		void tick();
		void exit();

	private:
		GLFWwindow* window;

		vector<RenderObject*> renderables;
};

extern Engine* engine;
