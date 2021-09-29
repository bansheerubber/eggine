#include <glad/gl.h>
#include "engine.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <glm/vec3.hpp>
#include <stdio.h>

#include "callbacks.h"
#include "../basic/renderContext.h"
#include "../resources/scriptFile.h"
#include "../util/time.h"
#include "torquescript.h"

Engine* engine = new Engine();

void Engine::initialize() {
	this->torquescript = tsCreateEngine(false);
	ts::torquescriptDefinitions();
	
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

	#ifdef EGGINE_DEBUG
	this->debugText = new Text("Arial", 12);
	this->debugText->color[0] = 0.0;
	this->debugText->color[1] = 1.0;
	this->debugText->color[2] = 0.0;
	#endif

	this->addKeybind(GLFW_KEY_EQUAL, binds::Keybind {
		"camera.zoomIn",
	});

	this->addKeybind(GLFW_KEY_MINUS, binds::Keybind {
		"camera.zoomOut",
	});

	// create camera once we're done with torquescript definitions
	this->camera = new Camera();

	this->registerBindPress("camera.zoomIn", this->camera);
	this->registerBindPress("camera.zoomOut", this->camera);
	this->registerBindRelease("camera.zoomIn", this->camera);
	this->registerBindRelease("camera.zoomOut", this->camera);

	// pre-load all .cs files
	engine->manager.loadResources(engine->manager.carton->database.get()->equals("extension", ".cs")->exec());

	// execute torquescript file
	resources::ScriptFile* mainCS = (resources::ScriptFile*)engine->manager.metadataToResources(
		engine->manager.carton->database.get()->equals("fileName", "scripts/main.cs")->exec()
	)[0];
	tsEval(this->torquescript, mainCS->script.c_str());
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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	#ifdef EGGINE_DEBUG
	this->debug.clearInfoMessages();
	this->debug.addInfoMessage(fmt::format("{} fps", (int)(1 / deltaTime)));
	this->debug.addInfoMessage(fmt::format("{0:05d} us for CPU render time", this->cpuRenderTime));
	this->debug.addInfoMessage(fmt::format("{0:05d} us for TS tick time", this->torquescriptTickTime));
	this->debug.addInfoMessage(fmt::format("{} renderables", this->renderables.head + this->renderableUIs.head));
	this->debug.addInfoMessage(fmt::format("{} zoom", this->camera->getZoom()));
	#endif

	if(deltaTime > 1.0) {
		deltaTime = 0;
	}

	glfwPollEvents();

	int escape = glfwGetKey(engine->window, GLFW_KEY_ESCAPE);
	if(escape) {
		return;
	}

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

	this->camera->see(deltaTime);
	this->ui.update();

	if(glfwWindowShouldClose(this->window)) {
		return;
	}

	RenderContext context = {
		camera: this->camera,
		ui: &this->ui,
	};

	// render everything
	long long startRenderTime = getMicrosecondsNow();
	for(size_t i = 0; i < this->renderables.head; i++) {
		this->renderables[i]->render(deltaTime, context);
	}

	#ifdef EGGINE_DEBUG
	this->debugText->setText(this->debug.getInfoText());
	#endif

	for(size_t i = 0; i < this->renderableUIs.head; i++) {
		this->renderableUIs[i]->render(deltaTime, context);
	}

	this->cpuRenderTime = getMicrosecondsNow() - startRenderTime;

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
