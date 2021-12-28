#include "engine.h"

#ifdef EGGINE_DEVELOPER_MODE
#include "imgui_impl_glfw.h"
#endif

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <glm/vec3.hpp>
#include <stdio.h>
#ifdef __switch__
#include <switch.h>
#endif

#include "callbacks.h"
#include "../test/developerGui.h"
#include "eggscript.h"
#include "../basic/renderContext.h"
#include "../resources/scriptFile.h"
#include "../util/time.h"

Engine* engine = new Engine();

void Engine::initialize() {
	// initialize nxlink and romfs right away
	#ifdef __switch__
	socketInitializeDefault();
	this->nxlink = nxlinkStdio();

	Result romfsResult = romfsInit();
	if(R_FAILED(romfsResult)) {
		printf("romfs failure: %d\n", romfsResult);
	}
	#endif

	FT_Init_FreeType(&this->ft);

	this->eggscript = esCreateEngine(false);
	es::eggscriptDefinitions();

	this->manager = new resources::ResourceManager(this->filePrefix + "out.carton");

	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".frag")->exec());
	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".vert")->exec());
	#ifdef __switch__
	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".dksh")->exec());
	#endif

	this->renderWindow.initialize();

	// initialize sound
	this->soundEngine.initialize();

	#ifndef __switch__
	glfwSetKeyCallback(this->renderWindow.window, onKeyPress);
	glfwSetMouseButtonCallback(this->renderWindow.window, onMousePress);
	glfwSetCursorPosCallback(this->renderWindow.window, onMouseMove);
	#endif

	#ifdef EGGINE_DEVELOPER_MODE
	glfwSetWindowFocusCallback(this->renderWindow.window, ImGui_ImplGlfw_WindowFocusCallback);
	glfwSetCursorEnterCallback(this->renderWindow.window, ImGui_ImplGlfw_CursorEnterCallback);
	glfwSetScrollCallback(this->renderWindow.window, ImGui_ImplGlfw_ScrollCallback);
	glfwSetCharCallback(this->renderWindow.window, ImGui_ImplGlfw_CharCallback);
	glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
	#endif

	#ifdef EGGINE_DEBUG
	this->debugText = new Text("Arial", 12);
	this->debugText->color[0] = 0.0;
	this->debugText->color[1] = 1.0;
	this->debugText->color[2] = 0.0;
	#endif

	// initialize keybinds
	#ifndef __switch__
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

	this->addKeybind(GLFW_KEY_W, binds::Keybind {
		"camera.up",
	});

	this->addKeybind(GLFW_KEY_S, binds::Keybind {
		"camera.down",
	});

	this->addKeybind(GLFW_KEY_A, binds::Keybind {
		"camera.left",
	});

	this->addKeybind(GLFW_KEY_D, binds::Keybind {
		"camera.right",
	});

	this->addMousebind(GLFW_MOUSE_BUTTON_LEFT, binds::Keybind {
		"chunk.mouseSelectTile",
	});

	this->addMousebind(GLFW_MOUSE_BUTTON_RIGHT, binds::Keybind {
		"chunk.mouseRightClickTile",
	});

	this->mouseToEnum["left-mouse-button"] = GLFW_MOUSE_BUTTON_LEFT;
	this->mouseToEnum["right-mouse-button"] = GLFW_MOUSE_BUTTON_RIGHT;
	this->mouseToEnum["middle-mouse-button"] = GLFW_MOUSE_BUTTON_MIDDLE;
	#endif

	this->gamepadToEnum["a-button"] = binds::A_BUTTON;
	this->gamepadToEnum["b-button"] = binds::B_BUTTON;
	this->gamepadToEnum["x-button"] = binds::X_BUTTON;
	this->gamepadToEnum["y-button"] = binds::Y_BUTTON;
	this->gamepadToEnum["d-pad-up"] = binds::D_PAD_UP;
	this->gamepadToEnum["d-pad-down"] = binds::D_PAD_DOWN;
	this->gamepadToEnum["d-pad-left"] = binds::D_PAD_LEFT;
	this->gamepadToEnum["d-pad-right"] = binds::D_PAD_RIGHT;
	this->gamepadToEnum["special-left"] = binds::SPECIAL_LEFT;
	this->gamepadToEnum["special-right"] = binds::SPECIAL_RIGHT;
	this->gamepadToEnum["left-button"] = binds::LEFT_BUTTON;
	this->gamepadToEnum["right-button"] = binds::RIGHT_BUTTON;
	this->gamepadToEnum["left-trigger"] = binds::LEFT_TRIGGER;
	this->gamepadToEnum["right-trigger"] = binds::RIGHT_TRIGGER;

	this->axesToEnum["left-axis-x"] = binds::LEFT_AXIS_X;
	this->axesToEnum["left-axis-y"] = binds::LEFT_AXIS_Y;
	this->axesToEnum["right-axis-y"] = binds::RIGHT_AXIS_X;
	this->axesToEnum["right-axis-y"] = binds::RIGHT_AXIS_Y;
	this->axesToEnum["mouse-axis-y"] = binds::MOUSE_AXIS_X;
	this->axesToEnum["mouse-axis-y"] = binds::MOUSE_AXIS_Y;

	this->addGamepadBind(binds::B_BUTTON, binds::Keybind {
		"chunk.selectTile"
	});

	this->addAxis(binds::LEFT_AXIS_X, binds::Keybind {
		"camera.xAxis"
	});

	this->addAxis(binds::LEFT_AXIS_Y, binds::Keybind {
		"camera.yAxis"
	});

	this->addAxis(binds::LEFT_AXIS_X, binds::Keybind {
		"chunk.xAxis"
	});

	this->addAxis(binds::LEFT_AXIS_Y, binds::Keybind {
		"chunk.yAxis"
	});

	this->addAxis(binds::MOUSE_AXIS_X, binds::Keybind {
		"chunk.mouseXAxis"
	});

	this->addAxis(binds::MOUSE_AXIS_Y, binds::Keybind {
		"chunk.mouseYAxis"
	});

	this->addAxis(binds::RIGHT_AXIS_Y, binds::Keybind {
		"camera.zoomAxis"
	});

	// create camera once we're done with eggscript definitions
	this->camera = new Camera();

	// pre-load all .egg files
	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".egg")->exec());

	// execute eggscript file
	resources::ScriptFile* mainCS = (resources::ScriptFile*)engine->manager->metadataToResources(
		engine->manager->carton->database.get()->equals("fileName", "scripts/main.egg")->exec()
	)[0];
	esExecFileFromContents(this->eggscript, "scripts/main.egg", mainCS->script.c_str());
	esCallFunction(this->eggscript, "init", 0, nullptr);

	this->renderWindow.initializeHTML();

	#ifdef EGGINE_DEVELOPER_MODE
	this->developerGui = new DeveloperGui();
	#endif
}

void Engine::exit() {
	FT_Done_FreeType(ft);

	this->renderWindow.deinitialize();

	#ifdef __switch__
	close(this->nxlink);
	romfsExit();
	socketExit();
	#endif
}

void Engine::tick() {
	start_tick:

	// handle client
	if(this->network.isClient() || this->network.isServer()) {
		this->network.receive();
	}

	this->soundEngine.tick(); // handle sounds

	int64_t startTime = getMicrosecondsNow();
	double deltaTime = (startTime - this->lastRenderTime) / 1000000.0;
	this->lastRenderTime = getMicrosecondsNow();

	this->renderWindow.prerender();

	#ifdef EGGINE_DEVELOPER_MODE
	this->developerGui->prerender();
	#endif

	#ifdef __switch__ // handle switch binds
	binds::Axes axes[4] = {binds::LEFT_AXIS_X, binds::LEFT_AXIS_Y, binds::RIGHT_AXIS_X, binds::RIGHT_AXIS_Y};
	int values[4] = {
		engine->renderWindow.leftStick.x,
		engine->renderWindow.leftStick.y,
		engine->renderWindow.rightStick.x,
		engine->renderWindow.rightStick.y
	};

	for(int i = 0; i < 4; i++) {
		binds::Axes axis = axes[i];
		float value = (float)values[i] / (float)(1 << 15);
		onAxisMove(axis, value);

		binds::GamepadButtons buttons[16] = {binds::B_BUTTON, binds::A_BUTTON, binds::Y_BUTTON, binds::X_BUTTON, binds::INVALID_BUTTON, binds::INVALID_BUTTON, binds::LEFT_BUTTON, binds::RIGHT_BUTTON, binds::LEFT_TRIGGER, binds::RIGHT_TRIGGER, binds::SPECIAL_RIGHT, binds::SPECIAL_LEFT, binds::D_PAD_LEFT, binds::D_PAD_UP, binds::D_PAD_RIGHT, binds::D_PAD_DOWN};
		for(uint64_t i = 0; i < 16; i++) {
			bool pressed = (this->renderWindow.buttons >> i) & 1;
			if(buttons[i] != binds::INVALID_BUTTON && ((this->lastGamepadButtons >> i) & 1) != pressed) {
				onGamepadButton(buttons[i], pressed);
			}
		}
		this->lastGamepadButtons = this->renderWindow.buttons;
	}
	#else // handle GLFW gamepads
	if(this->renderWindow.hasGamepad) {
		binds::Axes axes[4] = {binds::LEFT_AXIS_X, binds::LEFT_AXIS_Y, binds::RIGHT_AXIS_X, binds::RIGHT_AXIS_Y};
		for(int i = 0; i < 4; i++) {
			binds::Axes axis = axes[i];
			double value = engine->renderWindow.gamepad.axes[axis];

			if(axis == binds::LEFT_AXIS_Y || axis == binds::RIGHT_AXIS_Y) {
				value = -value;
			}

			if(abs(value) < 0.07) {
				value = 0.0;
			}

			onAxisMove(axis, value);
		}

		binds::GamepadButtons buttons[12] = {binds::A_BUTTON, binds::B_BUTTON, binds::X_BUTTON, binds::Y_BUTTON, binds::D_PAD_UP, binds::D_PAD_DOWN, binds::D_PAD_LEFT, binds::D_PAD_RIGHT, binds::SPECIAL_LEFT, binds::SPECIAL_RIGHT, binds::LEFT_BUTTON, binds::RIGHT_BUTTON};
		int glfwButtons[12] = {GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_B, GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_Y, GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_GAMEPAD_BUTTON_BACK, GLFW_GAMEPAD_BUTTON_START, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER};
		for(int i = 0; i < 12; i++) {
			unsigned char pressed = engine->renderWindow.gamepad.buttons[glfwButtons[i]];
			if(this->lastGamepadButtons[glfwButtons[i]] != pressed) {
				onGamepadButton(buttons[i], pressed);
			}
		}

		for(unsigned int i = 0; i < 15; i++) {
			this->lastGamepadButtons[i] = engine->renderWindow.gamepad.buttons[i];
		}
	}
	#endif
	
	#ifdef EGGINE_DEBUG
	#ifndef __switch__
	this->debug.flushGLDebugMessages();
	#endif
	
	this->debug.clearInfoMessages();
	this->debug.addInfoMessage(fmt::format("{} fps", (int)(1 / deltaTime)));
	this->debug.addInfoMessage(fmt::format("{0:05d} us for CPU render time", this->cpuRenderTime));
	this->debug.addInfoMessage(fmt::format("{0:05d} us for TS tick time", this->eggscriptTickTime));
	this->debug.addInfoMessage(fmt::format("{} renderables", this->renderables.head + this->renderableUIs.head));
	this->debug.addInfoMessage(fmt::format("{} zoom", this->camera->getZoom()));

	if(this->network.isClient()) {
		this->debug.addInfoMessage(fmt::format("Client: connected to {}", this->network.client.getIPAddress().toString()));
	}
	else if(this->network.isServer()) {
		this->debug.addInfoMessage(fmt::format("Server: hosting on {}", this->network.getHostIPAddress().toString()));
		this->debug.addInfoMessage(fmt::format("{} connections", this->network.getConnectionCount()));
	}

	#endif

	if(deltaTime > 1.0) {
		deltaTime = 0;
	}

	// this->hasGamepad = glfwGetGamepadState(GLFW_JOYSTICK_1, &this->gamepad);

	// int escape = glfwGetKey(engine->window, GLFW_KEY_ESCAPE);
	// if(escape || this->gamepad.buttons[GLFW_GAMEPAD_BUTTON_START]) {
	// 	this->exit();
	// 	return;
	// }

	// handle eggscript
	int64_t esStartTime = getMicrosecondsNow();
	esTick(this->eggscript);
	this->eggscriptTickTime = getMicrosecondsNow() - esStartTime;

	this->camera->see(deltaTime);
	this->ui.update();

	// if(glfwWindowShouldClose(this->window)) {
	// 	return;
	// }

	RenderContext context = {
		camera: this->camera,
		ui: &this->ui,
	};

	// render everything
	int64_t startRenderTime = getMicrosecondsNow();
	for(size_t i = 0; i < this->renderables.head; i++) {
		this->renderables[i]->render(deltaTime, context);
	}

	#ifdef EGGINE_DEBUG
	this->debugText->setText(this->debug.getInfoText());
	#endif

	for(size_t i = 0; i < this->renderableUIs.head; i++) {
		this->renderableUIs[i]->render(deltaTime, context);
	}

	#ifdef EGGINE_DEVELOPER_MODE
	this->developerGui->render();
	#endif

	this->cpuRenderTime = getMicrosecondsNow() - startRenderTime;

	this->renderWindow.render();

	if(this->network.isServer()) {
		this->network.tick();
	}

	goto start_tick;
}

void Engine::addRenderObject(RenderObject* renderable) {
	this->renderables[this->renderables.head] = renderable;
	this->renderables.pushed();
}

void Engine::removeRenderObject(RenderObject* renderable) {
	this->renderables.remove(renderable);
}

void Engine::addUIObject(RenderObject* renderable) {
	this->renderableUIs[this->renderableUIs.head] = renderable;
	this->renderableUIs.pushed();
}

void Engine::removeUIObject(RenderObject* renderable) {
	this->renderableUIs.remove(renderable);
}

void Engine::setFilePrefix(string filePrefix) {
	this->filePrefix = filePrefix;
}

string Engine::getFilePrefix() {
	return this->filePrefix;
}

void engineInitRenderables(class Engine*, RenderObject** object) {
	*object = nullptr;
}
