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
#include "basic/text.h"

#include "test/tileMath.h"

int main(int argc, char* argv[]) {
	engine->initialize();

	size_t size = 25;
	for(size_t i = 0; i < size * size; i++) {
		RenderTestContainer* container = new RenderTestContainer(tilemath::indexToCoordinate(i, size));
	}

	engine->tick();

	return 0;
}
