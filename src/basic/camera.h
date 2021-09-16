#pragma once

#include <glm/matrix.hpp>
#include <glm/vec2.hpp>

#include "gameObject.h"

class Camera : public GameObject {
	public:
		void see(double deltaTime);
		glm::mat4 projectionMatrix;

		void onBindPress(string &bind);
	
	private:
		glm::vec2 position = glm::vec2(0, 0);
		double zoom = 1;
};
