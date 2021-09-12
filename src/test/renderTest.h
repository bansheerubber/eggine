#pragma once

#include <glfw/glfw3.h>
#include <stdio.h>
#include <glm/vec2.hpp>
#include <vector>

#include "../basic/instancedRenderObject.h"

using namespace std;

class RenderTest : public InstancedRenderObject {
	public:
		RenderTest(class RenderTestContainer* container);
		void render(double deltaTime);
		void updateDisplayList(RenderTestContainer* container);

		glm::vec2 offset = glm::vec2(0, 0.2);
		glm::vec2 direction = glm::vec2(1, -1);
};
