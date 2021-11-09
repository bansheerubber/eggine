#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#endif

#include <array>
#include <tsl/robin_map.h>
#include <vector>

#include "memory.h"

using namespace std;

namespace std {
	template<>
	struct hash<pair<string, size_t>> {
		size_t operator()(pair<string, size_t> const& source) const noexcept {
			size_t result = hash<string>{}(source.first);
			return result ^ (source.second + 0x9e3779b9 + (result << 6) + (result >> 2));
    }
	};

	template<>
	struct equal_to<pair<string, size_t>> {
		bool operator()(const pair<string, size_t>& x, const pair<string, size_t>& y) const {
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
			void bindUniform(string uniformName, void* data, unsigned int size, size_t cacheIndex = 0, bool setOnce = false);
			void bindTexture(string uniformName, unsigned int texture);
		
		protected:
			vector<class Shader*> shaders;
			Window* window = nullptr;

			tsl::robin_map<string, unsigned int> uniformToBinding;

			#ifdef __switch__
			tsl::robin_map<string, switch_memory::Piece*> uniformToPiece;
			void createUniformMemory(string uniformName, unsigned int size);
			#else
			GLuint program = GL_INVALID_INDEX;
			tsl::robin_map< pair<string, size_t>, GLuint> uniformToBuffer;
			void createUniformBuffer(string uniformName, unsigned int size, size_t cacheIndex);
			#endif
	};
};
