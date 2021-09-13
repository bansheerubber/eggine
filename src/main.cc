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

int main(int argc, char* argv[]) {
	engine->initialize();

	RenderTestContainer container1(false);
	RenderTestContainer container2(true);

	engine->tick();

	return 0;
}
