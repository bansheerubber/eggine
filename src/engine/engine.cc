#include <glad/gl.h>
#include "engine.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <glm/vec3.hpp>
#include <stdio.h>

#include "callbacks.h"
#include "../basic/renderContext.h"
#include "../util/time.h"

Engine* engine = new Engine();

void Engine::initialize() {
	FT_Init_FreeType(&this->ft);

	if(!glfwInit()) {
		printf("failed to initialize glfw\n");
	}
	else {
		printf("initialized glfw\n");
	}

	this->windowWidth = 640;
	this->windowHeight = 480;

	this->window = glfwCreateWindow(this->windowWidth, this->windowHeight, "eggine", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetWindowSizeCallback(window, onWindowResize);
	glfwSwapInterval(1);

	this->debugText = new Text("Arial", 12);
	this->debugText->color[0] = 0.0;
	this->debugText->color[1] = 1.0;
	this->debugText->color[2] = 0.0;
}

void Engine::exit() {
	glfwTerminate();
	FT_Done_FreeType(ft);
}

void Engine::tick() {
	start_tick:

	long long startTime = getMicrosecondsNow();
	double deltaTime = (startTime - this->lastRenderTime) / 1000000.0;
	this->lastRenderTime = getMicrosecondsNow();

	this->debugText->text = to_string((int)(1 / deltaTime)) + " fps\n";
	this->debugText->text += fmt::format("{0:05d}", this->cpuRenderTime) + " us for CPU render time\n";
	this->debugText->text += to_string(this->renderables.head) + " renderables";

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(deltaTime > 1.0) {
		deltaTime = 0;
	}

	glfwPollEvents();

	this->camera.see();
	this->ui.update();

	if(glfwWindowShouldClose(this->window)) {
		return;
	}

	RenderContext context = {
		camera: &this->camera,
		ui: &this->ui,
	};

	// render everything
	for(size_t i = 0; i < this->renderables.head; i++) {
		this->renderables[i]->render(deltaTime, context);
	}
	this->cpuRenderTime = getMicrosecondsNow() - startTime;

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
