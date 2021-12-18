#include "healthbar.h"

#include "chunkContainer.h"
#include "../engine/engine.h"
#include "tileMath.h"

Healthbar::Healthbar() : RenderObject(false) {
	engine->addUIObject(this);
}

void Healthbar::setPercent(double percent) {
	this->percent = percent;
}

void Healthbar::setPosition(glm::uvec3 position) {
	this->position = position;
}

void Healthbar::render(double deltaTime, RenderContext &context) {
	double width = 0.9;
	double height = 0.1;
	double offset = 0.2;

	this->box.size.x = width;
	this->box.size.y = height;
	this->box.color = glm::vec4(0.2, 0.2, 0.2, 1);

	this->box.position = tilemath::tileToScreen(this->position, engine->chunkContainer->getRotation());
	this->box.position.x -= width / 2;
	this->box.position.y -= height / 2;

	this->box.position.y += offset;

	this->box.render(context.camera->projectionMatrix); // render the background

	this->box.size.x = width * this->percent;
	this->box.color = glm::vec4(1, 0, 0, 1);
	this->box.render(context.camera->projectionMatrix); // render the foreground
}
