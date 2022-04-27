#include "box.h"

#include "../engine/engine.h"
#include "shader.h"
#include "../resources/resourceManager.h"

render::Program* render::Box::Program = nullptr;
render::VertexBuffer* render::Box::Vertices = nullptr;
render::VertexAttributes* render::Box::VertexAttributes = nullptr;

render::Box::Box() {
	if(render::Box::Program == nullptr) {
		render::Shader* vertexShader = new render::Shader(&engine->renderWindow);
		vertexShader->load(getShaderSource("shaders/box.vert"), render::SHADER_VERTEX);

		render::Shader* fragmentShader = new render::Shader(&engine->renderWindow);
		fragmentShader->load(getShaderSource("shaders/box.frag"), render::SHADER_FRAGMENT);

		render::Box::Program = new render::Program(&engine->renderWindow);
		render::Box::Program->addShader(vertexShader);
		render::Box::Program->addShader(fragmentShader);
	}

	if(render::Box::Vertices == nullptr) {
		render::Box::Vertices = new render::VertexBuffer(&engine->renderWindow);
		render::Box::Vertices->setData((glm::vec2*)&render::Box::VerticesSource[0], sizeof(render::Box::VerticesSource), alignof(glm::vec2));
	}
	
	if(render::Box::VertexAttributes == nullptr) {
		render::Box::VertexAttributes = new render::VertexAttributes(&engine->renderWindow);
		render::Box::VertexAttributes->addVertexAttribute(render::Box::Vertices, 0, 2, render::VERTEX_ATTRIB_FLOAT, 0, sizeof(glm::vec2), 0);
	}
}

void render::Box::render(glm::mat4 projection) {
	struct VertexBlock {
		glm::mat4 projection;
		glm::vec2 position;
		glm::vec2 size;
	} vb;

	if(projection == glm::mat4(1)) {
		vb.projection = engine->ui.projectionMatrix;
	}
	else {
		vb.projection = projection;
	}
	vb.position = this->position;
	vb.size = this->size;

	struct FragmentBlock {
		glm::vec4 color;
	} fb;
	fb.color = this->color;

	engine->renderWindow.getState(0).bindProgram(render::Box::Program);
	engine->renderWindow.getState(0).bindUniform("vertexBlock", &vb, sizeof(vb));
	engine->renderWindow.getState(0).bindUniform("fragmentBlock", &fb, sizeof(fb));
	engine->renderWindow.getState(0).bindVertexAttributes(render::Box::VertexAttributes);
	engine->renderWindow.getState(0).draw(render::PRIMITIVE_TRIANGLE_STRIP, 0, 4, 0, 1);
}
