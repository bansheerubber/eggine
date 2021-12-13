#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "program.h"
#include "vertexAttributes.h"
#include "vertexBuffer.h"

namespace render {
	class Box {
		public:
			Box();
			void render(glm::mat4 projection = glm::mat4(1));

			glm::vec2 position;
			glm::vec2 size;
			glm::vec4 color;

		protected:
			static render::Program* Program;
			static render::VertexBuffer* Vertices;
			static render::VertexAttributes* VertexAttributes;
			static constexpr glm::vec2 VerticesSource[4] = {
				glm::vec2(0.0, 1.0),
				glm::vec2(1.0, 1.0),
				glm::vec2(0.0, 0.0),
				glm::vec2(1.0, 0.0),
			};
	};
};
