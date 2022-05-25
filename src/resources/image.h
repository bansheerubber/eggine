#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "../engine/console.h"
#include "../renderer/program.h"
#include "resourceObject.h"
#include "../renderer/texture.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

namespace resources {
	class Image: public ResourceObject {
		public:
			Image(ResourceManager &manager, carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize);

			glm::vec2 position;
			glm::vec2 size;
			
			void render();
			unsigned int getWidth();
			unsigned int getHeight();
			void reload(carton::Metadata* metadata, const unsigned char* buffer, uint64_t bufferSize) {
				ResourceObject::reload(metadata, buffer, bufferSize);
				console::warning("image reload not implemented\n");
			}
			unsigned int getBytesUsed() {
				return 0;
			}
		
		private:
		public:
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
