#include "line.h"

#include "camera.h"
#include "../engine/engine.h"
#include "../renderer/shader.h"

render::Program* Line::Program = nullptr;

Line::Line() {
	if(Line::Program == nullptr) {
		render::Shader* vertexShader = new render::Shader(&engine->renderWindow);
		vertexShader->load(getShaderSource("shaders/line.vert"), render::SHADER_VERTEX);

		render::Shader* fragmentShader = new render::Shader(&engine->renderWindow);
		fragmentShader->load(getShaderSource("shaders/line.frag"), render::SHADER_FRAGMENT);

		Line::Program = new render::Program(&engine->renderWindow);
		Line::Program->addShader(vertexShader);
		Line::Program->addShader(fragmentShader);
	}
	
	this->buffer = new render::VertexBuffer(&engine->renderWindow);
	this->buffer->setDynamicDraw(true);

	this->attributes = new render::VertexAttributes(&engine->renderWindow);
	this->attributes->addVertexAttribute(this->buffer, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
}

Line::~Line() {
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
	
	this->buffer->setData(this->positionBuffer, this->positions.size() * sizeof(glm::vec2), alignof(glm::vec2));
}

void Line::render(double deltaTime, RenderContext &context) {
	if(this->positions.size() > 0) {
		engine->renderWindow.getState(0).bindProgram(Line::Program);

		struct {
			glm::mat4 projection;
		} vb;
		vb.projection = context.camera->getProjectionMatrix();

		engine->renderWindow.getState(0).bindUniform("vertexBlock", &vb.projection[0][0], sizeof(glm::mat4));
		engine->renderWindow.getState(0).bindVertexAttributes(this->attributes);

		engine->renderWindow.getState(0).draw(render::PRIMITIVE_LINE_STRIP, 0, this->positions.size(), 0, 1);
	}
}
