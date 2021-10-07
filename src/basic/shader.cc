#include "../helpers.h"
#include GLAD_HEADER

#include "shader.h"

#include "../resources/shaderSource.h"

Shader::Shader(string vertexFileName, string fragmentFileName) {
	this->shaders = new GLuint[2]{GL_INVALID_INDEX, GL_INVALID_INDEX};

	Shader::CompileShader(
		&this->shaders[0],
		GL_VERTEX_SHADER,
		((resources::ShaderSource*)engine->manager->loadResources(
			engine->manager->carton->database.get()->equals("fileName", vertexFileName)->exec()
		)[0])->source.c_str()
	);

	Shader::CompileShader(
		&this->shaders[1],
		GL_FRAGMENT_SHADER,
		((resources::ShaderSource*)engine->manager->loadResources(
			engine->manager->carton->database.get()->equals("fileName", fragmentFileName)->exec()
		)[0])->source.c_str()
	);

	Shader::LinkProgram(&this->program, this->shaders, 2);
}

Shader::Shader(GLuint vertexShader, GLuint fragmentShader) {
	this->shaders = new GLuint[2];
	this->shaders[0] = vertexShader;
	this->shaders[1] = fragmentShader;

	Shader::LinkProgram(&this->program, this->shaders, 2);
}

void Shader::bind() {
	if(engine->boundShader != this && this->program != GL_INVALID_INDEX) {
		glUseProgram(this->program);
		engine->boundShader = this;
	}
}

bool Shader::isCompiled() {
	return this->program != GL_INVALID_INDEX;
}

GLuint Shader::getUniform(string name) {
	if(this->program == GL_INVALID_INDEX) {
		return 0; // return a value that won't destroy everything
	}
	
	auto it = this->uniformToLocation.find(name);
	if(it == this->uniformToLocation.end()) {
		GLuint uniform = glGetUniformLocation(this->program, name.c_str());
		this->uniformToLocation[name] = uniform;
		return uniform;
	}
	return it.value();
}

bool Shader::CompileShader(GLuint* destination, GLenum type, const char* source) {
	if(*destination != GL_INVALID_INDEX) {
		return true;
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
		return false;
	}
	else {
		*destination = shader;
	}
	return true;
}

bool Shader::LinkProgram(GLuint* destination, GLuint* shaders, GLuint shaderCount) {
	if(*destination != GL_INVALID_INDEX) {
		return true;
	}
	
	GLuint program = glCreateProgram();
	for(GLuint i = 0; i < shaderCount; i++) {
		if(shaders[i] == GL_INVALID_INDEX) {
			printf("shaders not compiled\n");
			return false;
		}
		
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
