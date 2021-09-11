#include <glad/gl.h>
#include "renderTest.h"

#include <stdio.h>

GLuint RenderTest::vertexShader = GL_INVALID_INDEX;
GLuint RenderTest::fragmentShader = GL_INVALID_INDEX;

RenderTest::RenderTest() {
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);

	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&RenderTest::vertexShader,
		#include "shaders/test.vert"	
	);

	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&RenderTest::fragmentShader,
		#include "shaders/test.vert"	
	);
}

void RenderTest::render() {

}
