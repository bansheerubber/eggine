#pragma once

#include <glfw/glfw3.h>
#include <glm/vec2.hpp>

#include "../basic/renderObject.h"

using namespace std;

class RenderTest : public RenderObject {
	public:
		RenderTest();
		void render(double deltaTime);

		glm::vec2 offset = glm::vec2(0, 0.2);
		glm::vec2 direction = glm::vec2(1, -1);
	
	private:
		GLuint vbo;
		GLuint vao;

		static GLuint* shaders;
		static GLuint shaderProgram;
		static GLuint* uniforms;

		float vertices[6] = {
			0.0f,  0.01f, // Vertex 1 (X, Y)
			0.01f, -0.01f, // Vertex 2 (X, Y)
			-0.01f, -0.01f  // Vertex 3 (X, Y)
		};
};
