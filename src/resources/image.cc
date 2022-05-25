#include "image.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../engine/engine.h"
#include "resourceManager.h"
#include "../renderer/shader.h"

render::Program* resources::Image::Program = nullptr;
render::VertexBuffer* resources::Image::Vertices = nullptr;
render::VertexBuffer* resources::Image::UVs = nullptr;
render::VertexAttributes* resources::Image::Attributes = nullptr;

resources::Image::Image(
	ResourceManager &manager,
	carton::Metadata* metadata,
	const unsigned char* buffer,
	uint64_t bufferSize
) : ResourceObject(manager, metadata) {
	this->texture = new render::Texture(&engine->renderWindow);
	this->texture->setFilters(render::TEXTURE_FILTER_NEAREST, render::TEXTURE_FILTER_NEAREST);
	this->texture->setWrap(render::TEXTURE_WRAP_CLAMP_TO_BORDER, render::TEXTURE_WRAP_CLAMP_TO_BORDER);
	this->texture->loadPNG(buffer, bufferSize);

	if(resources::Image::Program == nullptr) {
		render::Shader* vertexShader = new render::Shader(&engine->renderWindow);
		vertexShader->load(getShaderSource("shaders/image.vert"), render::SHADER_VERTEX);

		render::Shader* fragmentShader = new render::Shader(&engine->renderWindow);
		fragmentShader->load(getShaderSource("shaders/image.frag"), render::SHADER_FRAGMENT);

		resources::Image::Program = new render::Program(&engine->renderWindow);
		resources::Image::Program->addShader(vertexShader);
		resources::Image::Program->addShader(fragmentShader);
	}

	if(resources::Image::Vertices == nullptr) {
		resources::Image::Vertices = new render::VertexBuffer(&engine->renderWindow);
		resources::Image::Vertices->setData((glm::vec2*)resources::Image::VertexSource, sizeof(resources::Image::VertexSource), alignof(glm::vec2));

		resources::Image::UVs = new render::VertexBuffer(&engine->renderWindow);
		resources::Image::UVs->setData((glm::vec2*)resources::Image::UVSource, sizeof(resources::Image::UVSource), alignof(glm::vec2));

		resources::Image::Attributes = new render::VertexAttributes(&engine->renderWindow);
		resources::Image::Attributes->addVertexAttribute(resources::Image::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
		resources::Image::Attributes->addVertexAttribute(resources::Image::UVs, 1, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}
}

unsigned int resources::Image::getWidth() {
	return this->texture->getWidth();
}

unsigned int resources::Image::getHeight() {
	return this->texture->getHeight();
}

void resources::Image::render() {
	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 position;
		glm::vec2 size;
	} vb;
	vb.projection = engine->ui.getProjectionMatrix();
	vb.position = this->position;
	vb.size = this->size;

	engine->renderWindow.getState(0).bindProgram(resources::Image::Program);
	engine->renderWindow.getState(0).bindTexture("inTexture", this->texture);
	engine->renderWindow.getState(0).bindUniform("vertexBlock", &vb, sizeof(vb));

	engine->renderWindow.getState(0).bindVertexAttributes(resources::Image::Attributes);
	engine->renderWindow.getState(0).draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, 1);
}
