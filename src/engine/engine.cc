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
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	this->debugText = new Text("Arial", 12);
	this->debugText->color[0] = 0.0;
	this->debugText->color[1] = 1.0;
	this->debugText->color[2] = 0.0;
	#endif

	// initialize keybinds
	this->keyToScancode[" "] = GLFW_KEY_SPACE;
	this->keyToScancode["'"] = GLFW_KEY_APOSTROPHE;
	this->keyToScancode[","] = GLFW_KEY_COMMA;
	this->keyToScancode["-"] = GLFW_KEY_MINUS;
	this->keyToScancode["."] = GLFW_KEY_PERIOD;
	this->keyToScancode["/"] = GLFW_KEY_SLASH;
	this->keyToScancode["0"] = GLFW_KEY_0;
	this->keyToScancode["1"] = GLFW_KEY_1;
	this->keyToScancode["2"] = GLFW_KEY_2;
	this->keyToScancode["3"] = GLFW_KEY_3;
	this->keyToScancode["4"] = GLFW_KEY_4;
	this->keyToScancode["5"] = GLFW_KEY_5;
	this->keyToScancode["6"] = GLFW_KEY_6;
	this->keyToScancode["7"] = GLFW_KEY_7;
	this->keyToScancode["8"] = GLFW_KEY_8;
	this->keyToScancode["9"] = GLFW_KEY_9;
	this->keyToScancode[";"] = GLFW_KEY_SEMICOLON;
	this->keyToScancode["="] = GLFW_KEY_EQUAL;
	this->keyToScancode["a"] = GLFW_KEY_A;
	this->keyToScancode["b"] = GLFW_KEY_B;
	this->keyToScancode["c"] = GLFW_KEY_C;
	this->keyToScancode["d"] = GLFW_KEY_D;
	this->keyToScancode["e"] = GLFW_KEY_E;
	this->keyToScancode["f"] = GLFW_KEY_F;
	this->keyToScancode["g"] = GLFW_KEY_G;
	this->keyToScancode["h"] = GLFW_KEY_H;
	this->keyToScancode["i"] = GLFW_KEY_I;
	this->keyToScancode["j"] = GLFW_KEY_J;
	this->keyToScancode["k"] = GLFW_KEY_K;
	this->keyToScancode["l"] = GLFW_KEY_L;
	this->keyToScancode["m"] = GLFW_KEY_M;
	this->keyToScancode["n"] = GLFW_KEY_N;
	this->keyToScancode["o"] = GLFW_KEY_O;
	this->keyToScancode["p"] = GLFW_KEY_P;
	this->keyToScancode["q"] = GLFW_KEY_Q;
	this->keyToScancode["r"] = GLFW_KEY_R;
	this->keyToScancode["s"] = GLFW_KEY_S;
	this->keyToScancode["t"] = GLFW_KEY_T;
	this->keyToScancode["u"] = GLFW_KEY_U;
	this->keyToScancode["v"] = GLFW_KEY_V;
	this->keyToScancode["w"] = GLFW_KEY_W;
	this->keyToScancode["x"] = GLFW_KEY_X;
	this->keyToScancode["y"] = GLFW_KEY_Y;
	this->keyToScancode["z"] = GLFW_KEY_Z;
	this->keyToScancode["["] = GLFW_KEY_LEFT_BRACKET;
	this->keyToScancode["]"] = GLFW_KEY_RIGHT_BRACKET;
	this->keyToScancode["`"] = GLFW_KEY_GRAVE_ACCENT;
	this->keyToScancode["escape"] = GLFW_KEY_ESCAPE;
	this->keyToScancode["enter"] = GLFW_KEY_ENTER;
	this->keyToScancode["tab"] = GLFW_KEY_TAB;
	this->keyToScancode["backspace"] = GLFW_KEY_BACKSPACE;
	this->keyToScancode["insert"] = GLFW_KEY_INSERT;
	this->keyToScancode["delete"] = GLFW_KEY_DELETE;
	this->keyToScancode["right"] = GLFW_KEY_RIGHT;
	this->keyToScancode["left"] = GLFW_KEY_LEFT;
	this->keyToScancode["up"] = GLFW_KEY_UP;
	this->keyToScancode["down"] = GLFW_KEY_DOWN;
	this->keyToScancode["page up"] = GLFW_KEY_PAGE_UP;
	this->keyToScancode["page down"] = GLFW_KEY_PAGE_DOWN;
	this->keyToScancode["home"] = GLFW_KEY_HOME;
	this->keyToScancode["end"] = GLFW_KEY_END;
	this->keyToScancode["caps lock"] = GLFW_KEY_CAPS_LOCK;
	this->keyToScancode["scroll lock"] = GLFW_KEY_SCROLL_LOCK;
	this->keyToScancode["num lock"] = GLFW_KEY_NUM_LOCK;
	this->keyToScancode["print screen"] = GLFW_KEY_PRINT_SCREEN;
	this->keyToScancode["pause"] = GLFW_KEY_PAUSE;
	this->keyToScancode["f1"] = GLFW_KEY_F1;
	this->keyToScancode["f2"] = GLFW_KEY_F2;
	this->keyToScancode["f3"] = GLFW_KEY_F3;
	this->keyToScancode["f4"] = GLFW_KEY_F4;
	this->keyToScancode["f5"] = GLFW_KEY_F5;
	this->keyToScancode["f6"] = GLFW_KEY_F6;
	this->keyToScancode["f7"] = GLFW_KEY_F7;
	this->keyToScancode["f8"] = GLFW_KEY_F8;
	this->keyToScancode["f9"] = GLFW_KEY_F9;
	this->keyToScancode["f10"] = GLFW_KEY_F10;
	this->keyToScancode["f11"] = GLFW_KEY_F11;
	this->keyToScancode["f12"] = GLFW_KEY_F12;
	this->keyToScancode["f13"] = GLFW_KEY_F13;
	this->keyToScancode["f14"] = GLFW_KEY_F14;
	this->keyToScancode["f15"] = GLFW_KEY_F15;
	this->keyToScancode["f16"] = GLFW_KEY_F16;
	this->keyToScancode["f17"] = GLFW_KEY_F17;
	this->keyToScancode["f18"] = GLFW_KEY_F18;
	this->keyToScancode["f19"] = GLFW_KEY_F19;
	this->keyToScancode["f20"] = GLFW_KEY_F20;
	this->keyToScancode["f21"] = GLFW_KEY_F21;
	this->keyToScancode["f22"] = GLFW_KEY_F22;
	this->keyToScancode["f23"] = GLFW_KEY_F23;
	this->keyToScancode["f24"] = GLFW_KEY_F24;
	this->keyToScancode["left shirt"] = GLFW_KEY_LEFT_SHIFT;
	this->keyToScancode["left control"] = GLFW_KEY_LEFT_CONTROL;
	this->keyToScancode["left alt"] = GLFW_KEY_LEFT_ALT;
	this->keyToScancode["right shift"] = GLFW_KEY_RIGHT_SHIFT;
	this->keyToScancode["right control"] = GLFW_KEY_RIGHT_CONTROL;
	this->keyToScancode["right alt"] = GLFW_KEY_RIGHT_ALT;
	this->keyToScancode["menu"] = GLFW_KEY_MENU;

	for(auto &[key, scancode]: this->keyToScancode) {
		this->scancodeToKey[scancode] = key;
	}

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
	this->debug.flushGLDebugMessages();
	
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
