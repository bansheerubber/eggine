#pragma once

#ifdef __switch__
#include <deko3d.hpp>
#else
#include <GLFW/glfw3.h>
#endif

#include <tsl/robin_map.h>
#include "../resources/shaderSource.h"
#include <string>

#include "memory.h"

using namespace std;

namespace render {
	enum ShaderType {
		SHADER_FRAGMENT,
		SHADER_VERTEX
	};

	class Shader {
		friend class Program;
		
		public:
			Shader(class Window* window);

			void loadFromFile(string filename, ShaderType type);
			void load(string buffer, ShaderType type);
			void load(const char* buffer, uint64_t length, ShaderType type);
			void load(resources::ShaderSource* source, ShaderType type);
			void bind();

		protected:
			class Window* window;

			tsl::robin_map<string, unsigned int> uniformToBinding;
			ShaderType type;

			#ifdef __switch__
			switch_memory::Piece* memory = nullptr;
			dk::Shader shader;
			#else
			GLuint shader = GL_INVALID_INDEX;
			#endif

			void processUniforms(const char* buffer, uint64_t bufferSize);
			void processUniforms(string filename);
	};
};
