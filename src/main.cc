#include "main.h"

#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <random>
#include <stdio.h>

#include "engine/engine.h"
#include "basic/gameObject.h"
#include "test/renderTest.h"

int main(int argc, char* argv[]) {
	engine->initialize();
	for(int i = 0; i < 100000; i++) {
		RenderTest* object = new RenderTest();
		object->offset[0] = ((double) rand() / (RAND_MAX)) * 2 - 1;
		object->offset[1] = ((double) rand() / (RAND_MAX)) * 2 - 1;
	}

	engine->tick();

	return 0;
}
