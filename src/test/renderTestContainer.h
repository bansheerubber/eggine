#pragma once

#include <glfw/glfw3.h>

#include "../basic/instancedRenderObjectContainer.h"
#include "renderTest.h"

class RenderTestContainer : public InstancedRenderObjectContainer<RenderTest> {
	public:
		RenderTestContainer();

		void render(double deltaTime);

		glm::vec2 offsets[1000000];
	
	protected:
		GLuint shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
		GLuint shaderProgram = GL_INVALID_INDEX;
		GLuint vertexBufferObjects[2];
		GLuint vertexArrayObject;

		float vertices[6] = {
			0.0f,  0.01f, // Vertex 1 (X, Y)
			0.01f, -0.01f, // Vertex 2 (X, Y)
			-0.01f, -0.01f  // Vertex 3 (X, Y)
		};
};
