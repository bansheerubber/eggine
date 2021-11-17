#pragma once

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>

#include "font.h"
#include "../renderer/program.h"
#include "../renderer/texture.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"
#include "renderObject.h"

using namespace std;

class Text : public RenderObject {
	public:
		Text(bool addToUiList = true);
		Text(string family, int size, bool addToUiList = true);
		~Text();

		glm::vec3 color = glm::vec3(1, 1, 1);
		glm::vec2 position = glm::vec2(0, 0);
		Font* font = nullptr;

		void render(double deltaTime, RenderContext &context);
		void setText(string text);
		string getText();
	
	protected:

		render::VertexBuffer* vertexBuffers[2];
		render::VertexAttributes* vertexAttributes;
		string text;

		void updateBuffers();
		
		static render::Program* Program;
};
