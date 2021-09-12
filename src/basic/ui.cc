#include "ui.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

void UI::update() {
	double ratio = (double)engine->windowWidth / (double)engine->windowHeight;

	double viewportWidth = engine->windowWidth; // render in screen-space
	double viewportHeight = viewportWidth / ratio;
	
	this->projectionMatrix = glm::ortho(
		0.0,
		viewportWidth,
		viewportHeight,
		0.0
	);
}
