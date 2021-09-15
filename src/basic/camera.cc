#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

void Camera::see(double deltaTime) {
	int up = glfwGetKey(engine->window, 'W');
	int down = glfwGetKey(engine->window, 'S');
	int left = glfwGetKey(engine->window, 'A');
	int right = glfwGetKey(engine->window, 'D');

	float speed = 1.0f;
	this->position.x += (float)right * speed * deltaTime - (float)left * speed * deltaTime;
	this->position.y += (float)up * speed * deltaTime - (float)down * speed * deltaTime;
	
	double ratio = (double)engine->windowWidth / (double)engine->windowHeight;

	double viewportWidth = 25;
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
