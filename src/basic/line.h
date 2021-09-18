#pragma once

#include <glm/vec2.hpp>
#include <vector>

#include "renderObject.h"

using namespace std;

class Line: public RenderObject {
	public:
		Line();
		
		void addPosition(glm::vec2 position);
		void commit();

		void render(double deltaTime, RenderContext &context);
	
	private:
		static GLuint Shaders[];
		static GLuint Uniforms[];
		static GLuint ShaderProgram;
		
		GLuint vertexBufferObjects[1];
		GLuint vertexArrayObject;

		vector<glm::vec2> positions;

		glm::vec2* positionBuffer = nullptr;
};