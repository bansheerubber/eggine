#include <glad/gl.h>
#include "engine.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <glm/vec3.hpp>
#include <stdio.h>

#include <iostream>

#include "callbacks.h"
#include "../basic/renderContext.h"
#include "../util/time.h"

Engine* engine = new Engine();

void glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

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

	glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

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

	string text;
	text = to_string((int)(1 / deltaTime)) + " fps\n";
	text += fmt::format("{0:05d}", this->cpuRenderTime) + " us for CPU render time\n";
	text += to_string(this->renderables.head) + " renderables";
	this->debugText->setText(text);

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
