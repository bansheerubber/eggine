#pragma once

#include <glfw/glfw3.h>
#include <glm/vec3.hpp>

#include <tsl/robin_map.h>

#include "../basic/instancedRenderObjectContainer.h"
#include "../basic/pngImage.h"
#include "../basic/renderContext.h"
#include "renderTest.h"

class RenderTestContainer : public InstancedRenderObjectContainer<RenderTest> {
	public:
		RenderTestContainer(glm::vec2 position);

		void render(double deltaTime, RenderContext &context);

		glm::vec2* offsets = nullptr;
		int* textureIndices = nullptr;

		glm::vec2 position = glm::vec2(0, 0);
	
	protected:
		GLuint shaders[2] = {GL_INVALID_INDEX, GL_INVALID_INDEX};
		GLuint uniforms[2];
		GLuint shaderProgram = GL_INVALID_INDEX;
		GLuint vertexBufferObjects[4];
		GLuint vertexArrayObject;

		tsl::robin_map<int, tsl::robin_map<int, int>> tiles;

		static PNGImage* Image;
		static GLuint Texture;

		static constexpr int Size = 25;
		int height = 1;

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
