#include <glad/gl.h>
#include "renderTest.h"

#include <stdio.h>

GLuint* RenderTest::shaders = new GLuint[2]{ GL_INVALID_INDEX, GL_INVALID_INDEX };
GLuint RenderTest::shaderProgram = GL_INVALID_INDEX;
GLuint* RenderTest::uniforms = new GLuint[1];

RenderTest::RenderTest() {
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao); // fill out the VAO 

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // turn it off

	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&RenderTest::shaders[0],
		#include "shaders/test.vert"
	);

	RenderObject::CompileShader(
		GL_FRAGMENT_SHADER,
		&RenderTest::shaders[1],
		#include "shaders/test.frag"
	);

	if(RenderObject::LinkProgram(&RenderTest::shaderProgram, RenderTest::shaders, 2)) {
		RenderTest::uniforms[0] = glGetUniformLocation(RenderTest::shaderProgram, "offset");
	}

	glUseProgram(RenderTest::shaderProgram);
	glBindVertexArray(this->vao);
}

void RenderTest::render(double deltaTime) {
	this->offset[0] += this->direction[0] * deltaTime / 10.f;
	this->offset[1] += this->direction[1] * deltaTime / 10.f;

	if(this->offset[0] > 1) {
		this->direction[0] = -1;
	}
	else if(this->offset[0] < -1) {
		this->direction[0] = 1;
	}

	if(this->offset[1] > 1) {
		this->direction[1] = -1;
	}
	else if(this->offset[1] < -1) {
		this->direction[1] = 1;
	}

	glUniform2fv(RenderTest::uniforms[0], 1, &this->offset[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
