#include <glad/gl.h>
#include "renderObject.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>

void RenderObject::CompileShader(GLenum type, GLuint* destination, const char* source) {
	if(*destination != GL_INVALID_INDEX) {
		return;
	}
	
	GLuint shader = glCreateShader(type);
	const GLint size = strlen(source);
	glShaderSource(shader, 1, &source, &size);
	glCompileShader(shader);

	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(compiled == GL_FALSE) {
		// print the error log
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

		GLchar* log = new GLchar[logLength];
		glGetShaderInfoLog(shader, logLength, &logLength, log);

		glDeleteShader(shader);

		printf("failed to compile shader:\n%s\n", log);
		*destination = GL_INVALID_INDEX - 1;
		exit(1);
	}
	else {
		*destination = shader;
	}
}
