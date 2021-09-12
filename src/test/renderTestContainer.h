#pragma once

#include <glfw/glfw3.h>
#include <glm/vec3.hpp>

#include "../basic/instancedRenderObjectContainer.h"
#include "../basic/pngImage.h"
#include "../basic/renderContext.h"
#include "renderTest.h"

class RenderTestContainer : public InstancedRenderObjectContainer<RenderTest> {
	public:
		RenderTestContainer();

		void render(double deltaTime, RenderContext &context);

		glm::vec3 offsets[160000];
	
	protected:
		GLuint shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
		GLuint uniforms[2];
		GLuint shaderProgram = GL_INVALID_INDEX;
		GLuint vertexBufferObjects[3];
		GLuint vertexArrayObject;
		GLuint texture;
		PNGImage image = PNGImage("cube.png");

		float vertices[8] = {
			-0.03f,  0.06f,
			-0.03f, -0.06f,
			0.03f, 0.06f,
			0.03f, -0.06f
		};

		float uvs[8] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
};
