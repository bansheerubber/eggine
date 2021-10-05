#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <vector>

using namespace std;

#define EGGINE_DEBUG 1

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
		printf("OpenGL error (%u): ", this->id);
		
		switch(this->source) {
			case GL_DEBUG_SOURCE_API: {
				printf("source = api, ");
				break;
			}

			case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
				printf("source = window, ");
				break;
			}

			case GL_DEBUG_SOURCE_SHADER_COMPILER: {
				printf("source = shader compiler, ");
				break;
			}

			case GL_DEBUG_SOURCE_THIRD_PARTY: {
				printf("source = third party, ");
				break;
			}

			case GL_DEBUG_SOURCE_APPLICATION: {
				printf("source = application, ");
				break;
			}

			case GL_DEBUG_SOURCE_OTHER: {
				printf("source = other, ");
				break;
			}
		}

		switch(this->type) {
			case GL_DEBUG_TYPE_ERROR: {
				printf("type = error, ");
				break;
			}
			
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
				printf("type = deprecated, ");
				break;
			}

			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
				printf("type = undefined behavior, ");
				break;
			}

			case GL_DEBUG_TYPE_PORTABILITY: {
				printf("type = portability, ");
				break;
			}

			case GL_DEBUG_TYPE_PERFORMANCE: {
				printf("type = performance, ");
				break;
			}

			case GL_DEBUG_TYPE_MARKER: {
				printf("type = marker, ");
				break;
			}

			case GL_DEBUG_TYPE_PUSH_GROUP: {
				printf("type = push group, ");
				break;
			}

			case GL_DEBUG_TYPE_POP_GROUP: {
				printf("type = pop group, ");
				break;
			}

			case GL_DEBUG_TYPE_OTHER: {
				printf("type = other, ");
				break;
			}
		}
		
		switch(this->severity) {
			case GL_DEBUG_SEVERITY_HIGH: {
				printf("severity = high; ");
				break;
			}

			case GL_DEBUG_SEVERITY_MEDIUM: {
				printf("severity = medium; ");
				break;
			}

			case GL_DEBUG_SEVERITY_LOW: {
				printf("severity = low; ");
				break;
			}

			case GL_DEBUG_SEVERITY_NOTIFICATION: {
				printf("severity = notification; ");
				break;
			}
		}

		printf("'%.*s'\n", length, message);
	}
};

class Debug {
	friend class Engine;
	friend void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

	public:
		void addInfoMessage(string message);
	
	private:
		void logMessages();
		string getInfoText();
		void clearInfoMessages();
		void flushGLDebugMessages();
		vector<string> infoMessages;
		vector<GLDebugMessage> glDebugMessages;
};
