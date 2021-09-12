#pragma once

#include <glm/matrix.hpp>

#include "gameObject.h"

class Camera : public GameObject {
	public:
		void see();
		glm::mat4 projectionMatrix;
};
