#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

void Camera::see(double deltaTime) {
	this->zoomInTimer += deltaTime;
	this->zoomOutTimer += deltaTime;

	if(this->zoomInRepeating == 1 && this->zoomInTimer > 0.5) {
		this->zoomInRepeating = 2;
	}

	if(this->zoomOutRepeating == 1 && this->zoomOutTimer > 0.5) {
		this->zoomOutRepeating = 2;
	}
	
	if(this->zoomInRepeating == 2) {
		this->setZoomLevel(this->zoomLevel - deltaTime * 15.0f);
	}
	else if(this->zoomOutRepeating == 2) {
		this->setZoomLevel(this->zoomLevel + deltaTime * 15.0f);
	}
	
	double zoom = this->getZoom();
	int up = glfwGetKey(engine->window, 'W');
	int down = glfwGetKey(engine->window, 'S');
	int left = glfwGetKey(engine->window, 'A');
	int right = glfwGetKey(engine->window, 'D');

	float speed = deltaTime * 5.0f  + 0.05f / zoom;
	this->position.x += (float)right * speed - (float)left * speed;
	this->position.y += (float)up * speed - (float)down * speed;
	
	double ratio = (double)engine->windowWidth / (double)engine->windowHeight;

	double viewportWidth = 10 / zoom;
	double viewportHeight = viewportWidth / ratio;

	this->top = viewportHeight / 2.0f + this->position.y;
	this->right = viewportWidth / 2.0f + this->position.x;
	this->bottom = -viewportHeight / 2.0f + this->position.y;
	this->left = -viewportWidth / 2.0f + this->position.x;
	
	this->projectionMatrix = glm::ortho(
		this->left,
		this->right,
		this->bottom,
		this->top,
		-10.0,
		10.0
	);
}

void Camera::onBindPress(string &bind) {
	if(bind == "camera.zoomIn") {
		this->setZoomLevel(this->zoomLevel - 1.0f);
		this->zoomInRepeating = 1;
		this->zoomInTimer = 0;
	}
	else if(bind == "camera.zoomOut") {
		this->setZoomLevel(this->zoomLevel + 1.0f);
		this->zoomOutRepeating = 1;
		this->zoomOutTimer = 0;
	}
}

void Camera::onBindRelease(string &bind) {
	if(bind == "camera.zoomIn") {
		this->zoomInRepeating = 0;
	}
	else if(bind == "camera.zoomOut") {
		this->zoomOutRepeating = 0;
	}
}

void Camera::setZoomLevel(float zoomLevel) {
	this->zoomLevel = min(max(zoomLevel, this->minZoomLevel), this->maxZoomLevel);
}

float Camera::getZoom() {
	return 3.0f / this->zoomLevel;
}
