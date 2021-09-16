#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

void Camera::see(double deltaTime) {
	int up = glfwGetKey(engine->window, 'W');
	int down = glfwGetKey(engine->window, 'S');
	int left = glfwGetKey(engine->window, 'A');
	int right = glfwGetKey(engine->window, 'D');

	float speed = deltaTime * 10.0f / this->zoom;
	this->position.x += (float)right * speed - (float)left * speed;
	this->position.y += (float)up * speed - (float)down * speed;
	
	double ratio = (double)engine->windowWidth / (double)engine->windowHeight;

	double viewportWidth = 10 / this->zoom;
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

void Camera::onBindPress(string &bind) {
	if(bind == "camera.zoomIn") {
		this->zoom += this->zoom * 0.2;
	}
	else if(bind == "camera.zoomOut") {
		this->zoom += this->zoom * -0.2;
	}
}
