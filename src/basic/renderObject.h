#pragma once

#include <glfw/glfw3.h>

#include "gameObject.h"

class RenderObject : public GameObject {
	public:
		virtual void render() = 0;

		static void CompileShader(GLenum type, GLuint* destination, const char* source);
};
