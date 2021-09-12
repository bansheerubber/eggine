#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

void Camera::see() {
	double ratio = (double)engine->windowWidth / (double)engine->windowHeight;

	double viewportWidth = 1.f;
	double viewportHeight = viewportWidth / ratio;
	
	this->projectionMatrix = glm::ortho(
		-viewportWidth / 2.0f,
		viewportWidth / 2.0f,
		-viewportHeight / 2.0f,
		viewportHeight / 2.0f,
		-10.0,
		10.0
	);
}
