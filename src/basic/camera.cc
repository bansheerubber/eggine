#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

void Camera::see(double deltaTime) {
	int up = glfwGetKey(engine->window, 'W');
	int down = glfwGetKey(engine->window, 'S');
	int left = glfwGetKey(engine->window, 'A');
	int right = glfwGetKey(engine->window, 'D');

	this->position.x += (float)right * deltaTime - (float)left * deltaTime;
	this->position.y += (float)up * deltaTime - (float)down * deltaTime;
	
	double ratio = (double)engine->windowWidth / (double)engine->windowHeight;

	double viewportWidth = 1;
	double viewportHeight = viewportWidth / ratio;
	
	this->projectionMatrix = glm::ortho(
		-viewportWidth / 2.0f + this->position.x,
		viewportWidth / 2.0f + this->position.x,
		-viewportHeight / 2.0f + this->position.y,
		viewportHeight / 2.0f + this->position.y,
		-10.0,
		10.0
	);
}
