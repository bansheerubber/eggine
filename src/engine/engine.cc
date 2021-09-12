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
	long long frogtime = getMicrosecondsNow();
	auto end = this->renderables.end();
	for(auto it = this->renderables.begin(); it < end; ++it) {
		(*it)->render(deltaTime);
	}
	printf("%lld\n", getMicrosecondsNow() - frogtime);

	glfwSwapBuffers(window);

	goto start_tick;
}

void Engine::addRenderObject(RenderObject* renderable) {
	this->renderables.push_back(renderable);
}
