#pragma once

#include <glm/matrix.hpp>

#include "gameObject.h"

class UI : public GameObject {
	public:
		void update();
		glm::mat4 projectionMatrix;
};
