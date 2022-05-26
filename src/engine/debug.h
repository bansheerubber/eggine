#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#endif

#include <string>
#include <vector>

#include "../engine/console.h"

#define EGGINE_DEBUG 1

#ifndef __switch__
void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

struct GLDebugMessage {
	GLenum source;
	GLenum type;
	unsigned int id;
	GLenum severity;
	GLsizei length;
	const char* message;
	const void* parameter;

	void print() {
		console::error("OpenGL error (%u): ", this->id);
		
		switch(this->source) {
			case GL_DEBUG_SOURCE_API: {
				console::error("source = api, ");
				break;
			}

			case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
				console::error("source = window, ");
				break;
			}

			case GL_DEBUG_SOURCE_SHADER_COMPILER: {
				console::error("source = shader compiler, ");
				break;
			}

			case GL_DEBUG_SOURCE_THIRD_PARTY: {
				console::error("source = third party, ");
				break;
			}

			case GL_DEBUG_SOURCE_APPLICATION: {
				console::error("source = application, ");
				break;
			}

			case GL_DEBUG_SOURCE_OTHER: {
				console::error("source = other, ");
				break;
			}
		}

		switch(this->type) {
			case GL_DEBUG_TYPE_ERROR: {
				console::error("type = error, ");
				break;
			}
			
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
				console::error("type = deprecated, ");
				break;
			}

			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
				console::error("type = undefined behavior, ");
				break;
			}

			case GL_DEBUG_TYPE_PORTABILITY: {
				console::error("type = portability, ");
				break;
			}

			case GL_DEBUG_TYPE_PERFORMANCE: {
				console::error("type = performance, ");
				break;
			}

			case GL_DEBUG_TYPE_MARKER: {
				console::error("type = marker, ");
				break;
			}

			case GL_DEBUG_TYPE_PUSH_GROUP: {
				console::error("type = push group, ");
				break;
			}

			case GL_DEBUG_TYPE_POP_GROUP: {
				console::error("type = pop group, ");
				break;
			}

			case GL_DEBUG_TYPE_OTHER: {
				console::error("type = other, ");
				break;
			}
		}
		
		switch(this->severity) {
			case GL_DEBUG_SEVERITY_HIGH: {
				console::error("severity = high; ");
				break;
			}

			case GL_DEBUG_SEVERITY_MEDIUM: {
				console::error("severity = medium; ");
				break;
			}

			case GL_DEBUG_SEVERITY_LOW: {
				console::error("severity = low; ");
				break;
			}

			case GL_DEBUG_SEVERITY_NOTIFICATION: {
				console::error("severity = notification; ");
				break;
			}
		}

		console::error("'%.*s'\n", length, message);
	}
};
#endif

namespace render {
	class Program;
	class Texture;
	class VertexBuffer;
	class VertexAttributes;
};

class Debug {
	friend class Engine;
	#ifndef __switch__
	friend void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
	#endif

	public:
		~Debug();
		
		void addInfoMessage(std::string message);
		void drawTexture(render::Texture* texture);
	
	private:
		render::VertexBuffer* buffers[2];
		render::VertexAttributes* attributes;
		render::Program* program;
		std::vector<std::string> infoMessages;

		#ifndef __switch__
		std::vector<GLDebugMessage> glDebugMessages;
		void flushGLDebugMessages();
		#endif
		
		void logMessages();
		std::string getInfoText();
		void clearInfoMessages();

};
