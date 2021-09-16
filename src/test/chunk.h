#pragma once

#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <tsl/robin_map.h>

#include "../basic/instancedRenderObjectContainer.h"
#include "../basic/pngImage.h"
#include "../basic/renderContext.h"

class Tile;

class Chunk : public InstancedRenderObjectContainer<Tile> {
	public:
		Chunk(glm::vec2 position);

		void render(double deltaTime, RenderContext &context);

		glm::vec2* offsets = nullptr;
		int* textureIndices = nullptr;

		glm::ivec2 position = glm::vec2(0, 0);
		int height = 5;

		static constexpr int Size = 25;
		static GLuint Texture;
	
	protected:
		GLuint vertexBufferObjects[4];
		GLuint vertexArrayObject;

		tsl::robin_map<int, tsl::robin_map<int, int>> tiles;

		static PNGImage* Image;

		float vertices[8] = {
			-0.5f,  1.0f,
			-0.5f, -1.0f,
			0.5f, 1.0f,
			0.5f, -1.0f
		};

		float uvs[8] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
};
