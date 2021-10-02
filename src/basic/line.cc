#include <glad/gl.h>
#include "line.h"

#include "camera.h"
#include "shader.h"

GLuint Line::Shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
Shader* Line::Program = nullptr;

Line::Line() {
	glGenBuffers(1, this->vertexBufferObjects);
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObjects[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW); // don't allocate anything

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	Shader::CompileShader(
		&Line::Shaders[0],
		GL_VERTEX_SHADER,
		#include "shaders/line.vert"
	);

	Shader::CompileShader(
		&Line::Shaders[1],
		GL_FRAGMENT_SHADER,
		#include "shaders/line.frag"
	);

	Line::Program = new Shader(Line::Shaders[0], Line::Shaders[1]);
}

Line::~Line() {
	glDeleteBuffers(1, this->vertexBufferObjects);
	glDeleteVertexArrays(1, &this->vertexArrayObject);

	delete[] this->positionBuffer;
}

void Line::addPosition(glm::vec2 position) {
	this->positions.push_back(position);
}

void Line::commit() {
	if(this->positionBuffer != nullptr) {
		delete[] this->positionBuffer;
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
		Line::Program->bind();

		glUniformMatrix4fv(Line::Program->getUniform("projection"), 1, false, &context.camera->projectionMatrix[0][0]);
		
		glBindVertexArray(this->vertexArrayObject);
		glDrawArrays(GL_LINE_STRIP, 0, this->positions.size());
	}
}
