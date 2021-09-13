#pragma once

#include <string>

#include "font.h"
#include <glfw/glfw3.h>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "renderObject.h"

using namespace std;

class Text : public RenderObject {
	public:
		Text(string family, int size);
		void render(double deltaTime, RenderContext &context);
		glm::vec3 color = glm::vec3(1, 1, 1);
		glm::vec2 position = glm::vec2(0, 0);

		void setText(string text);
		string getText();
	
	protected:
		Font* font;

		GLuint vertexBufferObjects[2];
		GLuint vertexArrayObject;

		void updateBuffers();
		string text;
		
		static GLuint Shaders[2];
		static GLuint Uniforms[3];
		static GLuint ShaderProgram;
};
