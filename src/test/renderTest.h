#pragma once

#include <glfw/glfw3.h>

#include "../basic/renderObject.h"

using namespace std;

class RenderTest : public RenderObject {
	public:
		RenderTest();
		void render();
	
	private:
		GLuint vbo;

		static GLuint vertexShader;
		static GLuint fragmentShader;

		float vertices[6] = {
			0.0f,  0.5f, // Vertex 1 (X, Y)
			0.5f, -0.5f, // Vertex 2 (X, Y)
			-0.5f, -0.5f  // Vertex 3 (X, Y)
		};
};
