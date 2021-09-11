#include <glad/gl.h>
#include "engine.h"

#include <stdio.h>

#include "callbacks.h"

Engine* engine = new Engine();

void Engine::initialize() {
	if(!glfwInit()) {
		printf("failed to initialize glfw\n");
	}
	else {
		printf("initialized glfw\n");
	}

	this->window = glfwCreateWindow(640, 480, "eggine", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glfwSetWindowSizeCallback(window, onWindowResize);
	glfwSwapInterval(1);

	// this->tick();
}

void Engine::exit() {
	glfwTerminate();
}

void Engine::tick() {
	start_tick:

	glfwPollEvents();

	if(glfwWindowShouldClose(this->window)) {
		return;
	}

	// render everything
	auto end = this->renderables.end();
	for(auto it = this->renderables.begin(); it < end; ++it) {
		(*it)->render();
	}

	glfwSwapBuffers(window);

	goto start_tick;
}
