#include "debug.h"

#include "../util/cloneString.h"
#include "engine.h"
#include "../renderer/shader.h"

// void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
// 	if(id == 131185) {
// 		return;
// 	}
	
// 	#ifdef EGGINE_DEBUG
// 	engine->debug.glDebugMessages.push_back({
// 		source: source,
// 		type: type,
// 		id: id,
// 		severity: severity,
// 		length: length,
// 		message: cloneString(message),
// 		parameter: userParam,
// 	});
// 	#endif
// }

void Debug::addInfoMessage(string message) {
	this->infoMessages.push_back(message);
}

string Debug::getInfoText() {
	string output;
	for(string &message: this->infoMessages) {
		output += message + '\n';
	}
	return output;
}

void Debug::clearInfoMessages() {
	this->infoMessages.clear();
}

void Debug::flushGLDebugMessages() {
	// for(size_t i = 0; i < this->glDebugMessages.size(); i++) {
	// 	// only print groups if they actually captured some sort of message
	// 	if(
	// 		this->glDebugMessages[i].id == 1
	// 		&& i + 1 < this->glDebugMessages.size()
	// 		&& this->glDebugMessages[i + 1].id != 1
	// 	) {
	// 		this->glDebugMessages[i].print();
	// 	}
	// 	else if(this->glDebugMessages[i].id != 1) {
	// 		this->glDebugMessages[i].print();
	// 	}
	// 	else {
	// 		i++;
	// 	}
	// }

	// this->glDebugMessages.clear();
}

void Debug::drawTexture(render::Texture* texture) {
	if(this->program == nullptr) {
		render::Shader* vertexShader = new render::Shader(&engine->renderWindow);
		vertexShader->load(getShaderSource("shaders/texture.vert"), render::SHADER_VERTEX);

		render::Shader* fragmentShader = new render::Shader(&engine->renderWindow);
		fragmentShader->load(getShaderSource("shaders/texture.frag"), render::SHADER_FRAGMENT);

		this->program = new render::Program(&engine->renderWindow);
		this->program->addShader(vertexShader);
		this->program->addShader(fragmentShader);

		glm::vec2 triangleVertices[] = {
			glm::vec2(-0.5, 1.0),
			glm::vec2(-0.5, -1.0),
			glm::vec2(0.5, 1.0),
			glm::vec2(0.5, -1.0),
		};

		this->buffers[0] = new render::VertexBuffer(&engine->renderWindow);
		this->buffers[0]->setData(triangleVertices, sizeof(triangleVertices), alignof(glm::vec2));

		glm::vec2 triangleUVs[] = {
			glm::vec2(0.0, 0.0),
			glm::vec2(0.0, 1.0),
			glm::vec2(1.0, 0.0),
			glm::vec2(1.0, 1.0),
		};

		this->buffers[1] = new render::VertexBuffer(&engine->renderWindow);
		this->buffers[1]->setData(triangleUVs, sizeof(triangleUVs), alignof(glm::vec2));

		this->attributes = new render::VertexAttributes(&engine->renderWindow);
		this->attributes->addVertexAttribute(this->buffers[0], 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
		this->attributes->addVertexAttribute(this->buffers[1], 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}

	this->program->bind();
	texture->bind(0);
	this->program->bindTexture("inTexture", 0);
	this->attributes->bind();

	engine->renderWindow.draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, 1);
}
