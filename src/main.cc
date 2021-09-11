#include "main.h"

#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <stdio.h>

#include "engine/engine.h"
#include "basic/gameObject.h"
#include "test/renderTest.h"

int main(int argc, char* argv[]) {
	engine->initialize();
	RenderTest* renderTest = new RenderTest();
	engine->tick();

	return 0;
}
