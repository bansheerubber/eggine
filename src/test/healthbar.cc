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

	this->box.setSize(glm::vec2(width, height));
	this->box.setColor(glm::vec4(0.2, 0.2, 0.2, 1));

	glm::vec2 position = tilemath::tileToScreen(this->position, 0, engine->chunkContainer->getRotation());
	this->box.setPosition(glm::vec2(
		position.x - width / 2,
		position.y - height / 2 + offset
	));

	this->box.render(context.camera->getProjectionMatrix()); // render the background

	this->box.setSize(glm::vec2(width * this->percent, height));
	this->box.setColor(glm::vec4(1, 0, 0, 1));
	this->box.render(context.camera->getProjectionMatrix()); // render the foreground
}
