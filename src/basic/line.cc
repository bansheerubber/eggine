#include <glad/gl.h>
#include "line.h"

#include "camera.h"

GLuint Line::Shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
GLuint Line::Uniforms[1] = {GL_INVALID_INDEX};
GLuint Line::ShaderProgram = GL_INVALID_INDEX;

Line::Line() {
	glGenBuffers(1, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW); // don't allocate anything

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	RenderObject::CompileShader(
		GL_VERTEX_SHADER,
		&Line::Shaders[0],
		#include "shaders/line.vert"
	);

	RenderObject::CompileShader(
		GL_FRAGMENT_SHADER,
		&Line::Shaders[1],
		#include "shaders/line.frag"
	);

	if(RenderObject::LinkProgram(&Line::ShaderProgram, Line::Shaders, 2)) {
		Line::Uniforms[0] = glGetUniformLocation(Line::ShaderProgram, "projection");
	}
}

void Line::addPosition(glm::vec2 position) {
	this->positions.push_back(position);
}

void Line::commit() {
	if(this->positionBuffer != nullptr) {
		delete this->positionBuffer;
	}

	this->positionBuffer = new glm::vec2[this->positions.size()];
	int i = 0;
	for(glm::vec2 position: this->positions) {
		this->positionBuffer[i] = position;
		i++;
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, this->positions.size() * sizeof(glm::vec2), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->positions.size() * sizeof(glm::vec2), this->positionBuffer);
}

void Line::render(double deltaTime, RenderContext &context) {
	if(this->positions.size() > 0) {
		glUseProgram(Line::ShaderProgram);

		glUniformMatrix4fv(Line::Uniforms[0], 1, false, &context.camera->projectionMatrix[0][0]);
		
		glBindVertexArray(this->vertexArrayObject);
		glDrawArrays(GL_LINE_STRIP, 0, this->positions.size());
	}
}
