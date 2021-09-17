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
		glm::vec2 screenSpacePosition;
		int height = 5;

		static constexpr int Size = 25;
		static GLuint Texture;
	
	protected:
		void buildDebugLines();
		void defineBounds();
		
		GLuint vertexBufferObjects[1];
		GLuint vertexArrayObject;

		class Line* debugLine = nullptr;

		tsl::robin_map<int, tsl::robin_map<int, int>> tiles;

		double top = 0, right = 0, bottom = 0, left = 0;

		static PNGImage* Image;
		static glm::vec2 Offsets[];
		static GLuint VertexBufferObjects[];

		static constexpr float Vertices[8] = {
			-0.5f,  1.0f,
			-0.5f, -1.0f,
			0.5f, 1.0f,
			0.5f, -1.0f
		};

		static constexpr float UVs[8] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
};
