#pragma once

#include <glfw/glfw3.h>

#include "gameObject.h"

class RenderObject : public GameObject {
	public:
		RenderObject();
		
		virtual void render(double deltaTime) = 0;

		static void CompileShader(GLenum type, GLuint* destination, const char* source);
		static bool LinkProgram(GLuint* destination, GLuint* shaders, GLuint shaderCount);
};
