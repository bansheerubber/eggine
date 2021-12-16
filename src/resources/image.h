#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "../renderer/program.h"
#include "resourceObject.h"
#include "../renderer/texture.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

namespace resources {
	class Image: public ResourceObject {
		public:
			Image(class ResourceManager* manager, carton::Metadata* metadata, const unsigned char* buffer, size_t bufferSize);

			glm::vec2 position;
			glm::vec2 size;
			
			void render();
			unsigned int getWidth();
			unsigned int getHeight();
			unsigned int getBytesUsed() {
				return 0;
			}
		
		private:
			render::Texture* texture = nullptr;

			static render::Program* Program;
			static render::VertexBuffer* Vertices;
			static render::VertexBuffer* UVs;
			static render::VertexAttributes* Attributes;

			static constexpr glm::vec2 VertexSource[] = {
				glm::vec2(0.0, 1.0),
				glm::vec2(1.0, 1.0),
				glm::vec2(0.0, 0.0),
				glm::vec2(1.0, 0.0),
			};

			static constexpr glm::vec2 UVSource[] = {
				glm::vec2(0.0, 1.0),
				glm::vec2(1.0, 1.0),
				glm::vec2(0.0, 0.0),
				glm::vec2(1.0, 0.0),
			};
	};
};
