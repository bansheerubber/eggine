#include <glad/gl.h>
#include "renderObject.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "../engine/engine.h"

RenderObject::RenderObject() {
	engine->addRenderObject(this);
}

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

		printf("failed to compile shader:\n%.*s\n", logLength, log);
		*destination = GL_INVALID_INDEX - 1;
		exit(1);
	}
	else {
		*destination = shader;
	}
}

bool RenderObject::LinkProgram(GLuint* destination, GLuint* shaders, GLuint shaderCount) {
	if(*destination != GL_INVALID_INDEX) {
		return false;
	}
	
	GLuint program = glCreateProgram();
	for(GLuint i = 0; i < shaderCount; i++) {
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);

	GLint linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(linked == GL_FALSE) {
		// print the error log
		GLint logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

		GLchar* log = new GLchar[logLength];
		glGetProgramInfoLog(program, logLength, &logLength, log);

		glDeleteProgram(program);
		printf("failed to link program:\n%.*s\n", logLength, log);
		
		*destination = GL_INVALID_INDEX - 1;
		return false;
	}
	else {
		*destination = program;
	}

	for(GLuint i = 0; i < shaderCount; i++) {
		glDetachShader(program, shaders[i]);
	}

	return true;
}
