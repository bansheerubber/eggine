#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#endif

#include <array>
#include <tsl/robin_map.h>
#include <vector>

#include "memory.h"

namespace std {
	template<>
	struct hash<pair<std::string, uint64_t>> {
		size_t operator()(pair<std::string, uint64_t> const& source) const noexcept {
			uint64_t result = hash<std::string>{}(source.first);
			return result ^ (source.second + 0x9e3779b9 + (result << 6) + (result >> 2));
    }
	};

	template<>
	struct equal_to<pair<std::string, uint64_t>> {
		bool operator()(const pair<std::string, uint64_t>& x, const pair<std::string, uint64_t>& y) const {
			return x.first == y.first && x.second == y.second;
		}
	};
};

namespace render {
	class Program {
		public:
			Program(class Window* window);
			void bind();
			void addShader(class Shader* shader);
			void bindUniform(std::string uniformNamde, void* data, unsigned int size, uint64_t cacheIndex = 0, bool setOnce = false);
			void bindTexture(std::string uniformName, unsigned int texture);
		
		protected:
			std::vector<class Shader*> shaders;
			Window* window = nullptr;

			tsl::robin_map<std::string, unsigned int> uniformToBinding;

			#ifdef __switch__
			tsl::robin_map<std::string, switch_memory::Piece*> uniformToPiece;
			void createUniformMemory(std::string uniformName, unsigned int size);
			#else
			GLuint program = GL_INVALID_INDEX;
			tsl::robin_map<std::pair<std::string, uint64_t>, GLuint> uniformToBuffer;
			static unsigned int UniformCount;
			void createUniformBuffer(std::string uniformName, unsigned int size, uint64_t cacheIndex);
			#endif
	};
};
