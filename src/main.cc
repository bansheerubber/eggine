#include "main.h"

#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <random>
#include <stdio.h>

#include "engine/engine.h"
#include "basic/gameObject.h"
#include "basic/pngImage.h"
#include "test/renderTest.h"
#include "test/renderTestContainer.h"

int main(int argc, char* argv[]) {
	engine->initialize();

	PNGImage image = PNGImage("cube.png");

	RenderTestContainer container;
	for(int i = 0; i < 1000000; i++) {
		RenderTest* object = new RenderTest(&container);
		object->offset[0] = ((double)rand() / (RAND_MAX)) * 2 - 1;
		object->offset[1] = ((double)rand() / (RAND_MAX)) * 2 - 1;
	}

	engine->tick();

	return 0;
}
