#pragma once

#include <glm/vec2.hpp>

#include "../renderer/box.h"
#include "../basic/renderContext.h"
#include "../basic/renderObject.h"

class Healthbar: public RenderObject {
	public:
		Healthbar();
		void render(double deltaTime, RenderContext &context);
		void setPercent(double percent);
		void setPosition(glm::vec2 position);
	
	private:
		double percent = 1.0;
		glm::vec2 position;
		render::Box* box;
};
