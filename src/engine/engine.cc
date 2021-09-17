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
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetWindowSizeCallback(window, onWindowResize);
	glfwSetKeyCallback(window, onKeyPress);

	glfwSwapInterval(1);

	this->debugText = new Text("Arial", 12);
	this->debugText->color[0] = 0.0;
	this->debugText->color[1] = 1.0;
	this->debugText->color[2] = 0.0;

	this->addKeybind(GLFW_KEY_EQUAL, binds::Keybind {
		"camera.zoomIn",
	});

	this->addKeybind(GLFW_KEY_MINUS, binds::Keybind {
		"camera.zoomOut",
	});

	this->registerBindPress("camera.zoomIn", &this->camera);
	this->registerBindPress("camera.zoomOut", &this->camera);
	this->registerBindRelease("camera.zoomIn", &this->camera);
	this->registerBindRelease("camera.zoomOut", &this->camera);

	this->torquescript = tsCreateEngine(false);
	tsExecFile(this->torquescript, "main.cs");
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

	string text;
	text = to_string((int)(1 / deltaTime)) + " fps\n";
	text += fmt::format("{0:05d}", this->cpuRenderTime) + " us for CPU render time\n";
	text += fmt::format("{0:05d}", this->torquescriptTickTime) + " us for TS tick time\n";
	text += to_string(this->renderables.head + this->renderableUIs.head) + " renderables\n";
	text += to_string(this->camera.getZoom()) + " zoom\n";
	this->debugText->setText(text);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(deltaTime > 1.0) {
		deltaTime = 0;
	}

	glfwPollEvents();

	// handle torquescript
	long long tsStartTime = getMicrosecondsNow();
	tsTick(this->torquescript);
	this->torquescriptTickTime = getMicrosecondsNow() - tsStartTime;

	// handle keybinds before we do anything else
	if(this->heldEvents.size() > 0) {
		for(auto &event: this->heldEvents) {
			event.first->onBindHeld(event.second, deltaTime);
		}
		this->heldEvents.clear();
	}

	this->camera.see(deltaTime);
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

	for(size_t i = 0; i < this->renderableUIs.head; i++) {
		this->renderableUIs[i]->render(deltaTime, context);
	}

	this->cpuRenderTime = getMicrosecondsNow() - startTime;

	glfwSwapBuffers(window);

	goto start_tick;
}

void Engine::addRenderObject(RenderObject* renderable) {
	this->renderables[this->renderables.head] = renderable;
	this->renderables.pushed();
}

void Engine::addUIObject(RenderObject* renderable) {
	this->renderableUIs[this->renderableUIs.head] = renderable;
	this->renderableUIs.pushed();
}

void engineInitRenderables(class Engine*, RenderObject** object) {
	*object = nullptr;
}
