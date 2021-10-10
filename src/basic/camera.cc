#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

Camera::Camera() {
	this->reference = esCreateObject(engine->eggscript, "Camera", this);
}

Camera::~Camera() {
	esDeleteObject(this->reference);
}

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

	// handle keyboard
	int up = glfwGetKey(engine->window, 'W');
	int down = glfwGetKey(engine->window, 'S');
	int left = glfwGetKey(engine->window, 'A');
	int right = glfwGetKey(engine->window, 'D');

	float speed = deltaTime * 5.0f  + 0.05f / zoom;
	this->position.x += (float)right * speed - (float)left * speed;
	this->position.y += (float)up * speed - (float)down * speed;

	// handle gamepad
	if(engine->hasGamepad) {
		this->position.x += engine->gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_X] * speed;
		this->position.y -= engine->gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] * speed;
	}
	
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

void Camera::setPosition(glm::vec2 position) {
	this->position = position;
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

// eggscript bindings
void es::defineCamera() {
	esRegisterNamespace(engine->eggscript, "Camera");
	esNamespaceInherit(engine->eggscript, "SimObject", "Camera");

	esRegisterFunction(engine->eggscript, ES_ENTRY_OBJECT, es::getActiveCamera, "getActiveCamera", 0, nullptr);
	esEntryType setPositionArguments[3] = {ES_ENTRY_OBJECT, ES_ENTRY_NUMBER, ES_ENTRY_NUMBER};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::Camera__setPosition, "Camera", "setPosition", 3, setPositionArguments);
}

esEntryPtr es::getActiveCamera(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	esEntryPtr entry = new esEntry();
	entry->type = ES_ENTRY_OBJECT;
	entry->objectData = engine->camera->reference;
	return entry;
}

esEntryPtr es::Camera__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 3 && esCompareNamespaceToObject(args[0].objectData, "Camera")) {
		((Camera*)args[0].objectData->objectWrapper->data)->setPosition(glm::vec2(args[1].numberData, args[2].numberData));
	}
	
	return nullptr;
}
