#pragma once

#include <GLFW/glfw3.h>
#include <tsl/robin_map.h>
#include <string>

#include "../engine/engine.h"

using namespace std;

class Shader {
	public:
		Shader(string vertexFileName, string fragmentFileName); // loaded from .carton
		Shader(GLuint vertexShader, GLuint fragmentShader);

		void bind();
		bool isCompiled();
		GLuint getUniform(string name);

		static bool CompileShader(GLuint* destination, GLenum type, const char* source);
		static bool LinkProgram(GLuint* destination, GLuint* shaders, GLuint shaderCount);
	
	private:
		GLuint* shaders = nullptr;
		GLuint program = GL_INVALID_INDEX;

		tsl::robin_map<string, GLuint> uniformToLocation;
};
