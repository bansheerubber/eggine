#include <glad/gl.h>
#include "engine.h"

#include <stdio.h>

#include "callbacks.h"
#include "../util/time.h"

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
}

void Engine::exit() {
	glfwTerminate();
}

void Engine::tick() {
	start_tick:

	long long startTime = getMicrosecondsNow();
	double deltaTime = (startTime - this->lastRenderTime) / 100000.0;
	this->lastRenderTime = getMicrosecondsNow();

	glClear(GL_COLOR_BUFFER_BIT);

	if(deltaTime > 1.0) {
		deltaTime = 0;
	}

	glfwPollEvents();

	if(glfwWindowShouldClose(this->window)) {
		return;
	}

	// render everything
	for(size_t i = 0; i < this->renderables.head; i++) {
		this->renderables[i]->render(deltaTime);
	}

	glfwSwapBuffers(window);

	goto start_tick;
}

void Engine::addRenderObject(RenderObject* renderable) {
	this->renderables[this->renderables.head] = renderable;
	this->renderables.pushed();
}

void engineInitRenderables(class Engine*, RenderObject** object) {
	*object = nullptr;
}
