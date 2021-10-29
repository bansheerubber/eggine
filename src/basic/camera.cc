#include "camera.h"

#include <math.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"

Camera::Camera() {
	this->reference = esInstantiateObject(engine->eggscript, "Camera", this);

	engine->registerBind("camera.zoomIn", this);
	engine->registerBind("camera.zoomOut", this);
	engine->registerBind("camera.up", this);
	engine->registerBind("camera.down", this);
	engine->registerBind("camera.left", this);
	engine->registerBind("camera.right", this);

	engine->registerBindAxis("camera.xAxis", this);
	engine->registerBindAxis("camera.yAxis", this);
	engine->registerBindAxis("camera.zoomAxis", this);
}

Camera::~Camera() {
	esDeleteObject(this->reference);
}

glm::vec2 Camera::getViewport() {
	double ratio = (double)engine->renderWindow.width / (double)engine->renderWindow.height;
	double width = 10 / this->getZoom();
	return glm::vec2(width, width / ratio);
}

void Camera::see(double deltaTime) {
	this->keyMapping.zoomInTimer += deltaTime;
	this->keyMapping.zoomOutTimer += deltaTime;

	if(this->keyMapping.zoomInRepeating == 1 && this->keyMapping.zoomInTimer > 0.5) {
		this->keyMapping.zoomInRepeating = 2;
	}

	if(this->keyMapping.zoomOutRepeating == 1 && this->keyMapping.zoomOutTimer > 0.5) {
		this->keyMapping.zoomOutRepeating = 2;
	}
	
	if(this->keyMapping.zoomInRepeating == 2) {
		this->setZoomLevel(this->zoomLevel - deltaTime * 15.0f);
	}
	else if(this->keyMapping.zoomOutRepeating == 2) {
		this->setZoomLevel(this->zoomLevel + deltaTime * 15.0f);
	}

	if(abs(this->keyMapping.zoomAxis) > 0.1) {
		this->setZoomLevel(this->zoomLevel + deltaTime * 10.0f * -this->keyMapping.zoomAxis);
	}
	
	double zoom = this->getZoom();

	float speed = deltaTime * 5.0f  + 0.05f / zoom;
	this->position.x += (float)this->keyMapping.right * speed - (float)this->keyMapping.left * speed;
	this->position.y += (float)this->keyMapping.up * speed - (float)this->keyMapping.down * speed;

	this->position.x += this->keyMapping.xAxis * speed;
	this->position.y += this->keyMapping.yAxis * speed;

	glm::vec2 viewport = this->getViewport();

	this->top = viewport.y + this->position.y;
	this->right = viewport.x + this->position.x;
	this->bottom = -viewport.y + this->position.y;
	this->left = -viewport.x + this->position.x;
	
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

glm::vec2 Camera::getPosition() {
	return this->position;
}

glm::vec2 Camera::mouseToWorld(glm::vec2 mouse) {
	glm::vec2 viewport = this->getViewport();
	float halfWidth = (float)engine->renderWindow.width / 2.0f;
	float halfHeight = (float)engine->renderWindow.height / 2.0f;
	return glm::vec2(
		(mouse.x - halfWidth) / halfWidth * viewport.x + this->position.x + 0.5f,
		(mouse.y - halfHeight) / halfHeight * viewport.y - this->position.y - 12.0f / 64.0f
	);
}

void Camera::onBind(string &bind, binds::Action action) {
	if(bind == "camera.zoomIn") {
		if(action == binds::PRESS) {
			this->setZoomLevel(this->zoomLevel - 1.0f);
			this->keyMapping.zoomInRepeating = 1;
			this->keyMapping.zoomInTimer = 0;
		}
		else {
			this->keyMapping.zoomInRepeating = 0;
		}
	}
	else if(bind == "camera.zoomOut") {
		if(action == binds::PRESS) {
			this->setZoomLevel(this->zoomLevel + 1.0f);
			this->keyMapping.zoomOutRepeating = 1;
			this->keyMapping.zoomOutTimer = 0;
		}
		else {
			this->keyMapping.zoomOutRepeating = 0;
		}
	}
	else if(bind == "camera.up") {
		this->keyMapping.up = action == binds::PRESS;
	}
	else if(bind == "camera.down") {
		this->keyMapping.down = action == binds::PRESS;
	}
	else if(bind == "camera.left") {
		this->keyMapping.left = action == binds::PRESS;
	}
	else if(bind == "camera.right") {
		this->keyMapping.right = action == binds::PRESS;
	}
}

void Camera::onAxis(string &bind, double value) {
	if(bind == "camera.xAxis") {
		this->keyMapping.xAxis = value;
	}
	else if(bind == "camera.yAxis") {
		this->keyMapping.yAxis = value;
	}
	else if(bind == "camera.zoomAxis") {
		this->keyMapping.zoomAxis = value;
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

	esEntryType setPositionArguments[2] = {ES_ENTRY_OBJECT, ES_ENTRY_MATRIX};
	esRegisterMethod(engine->eggscript, ES_ENTRY_INVALID, es::Camera__setPosition, "Camera", "setPosition", 2, setPositionArguments);

	esEntryType getPositionArguments[1] = {ES_ENTRY_OBJECT};
	esRegisterMethod(engine->eggscript, ES_ENTRY_MATRIX, es::Camera__getPosition, "Camera", "getPosition", 1, getPositionArguments);
}

esEntryPtr es::getActiveCamera(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	return esCreateObject(esCloneObjectReference(engine->camera->reference));
}

esEntryPtr es::Camera__setPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 2 && esCompareNamespaceToObject(args[0].objectData, "Camera") && args[1].matrixData->rows == 2 && args[1].matrixData->columns == 1) {
		((Camera*)args[0].objectData->objectWrapper->data)->setPosition(
			glm::vec2(args[1].matrixData->data[0][0].numberData, args[1].matrixData->data[1][0].numberData)
		);
	}
	
	return nullptr;
}

esEntryPtr es::Camera__getPosition(esEnginePtr esEngine, unsigned int argc, esEntry* args) {
	if(argc == 1 && esCompareNamespaceToObject(args[0].objectData, "Camera")) {
		glm::vec2 position = ((Camera*)args[0].objectData->objectWrapper->data)->getPosition();
		return esCreateVector(2, (double)position.x, (double)position.y);
	}

	return nullptr;
}
