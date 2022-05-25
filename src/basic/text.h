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

class Text : public RenderObject {
	public:
		Text(bool addToUiList = true);
		Text(std::string family, int size, bool addToUiList = true);
		~Text();

		void render(double deltaTime, RenderContext &context);
		void setText(std::string text);
		std::string getText();

		void setColor(glm::vec3 color);
		glm::vec3 getColor();

		void setPosition(glm::vec2 position);
		glm::vec2 getPosition();

		void setFont(render::Font* font);
		render::Font* getFont();
	
	protected:
		render::VertexBuffer* vertexBuffers[2];
		render::VertexAttributes* vertexAttributes;
		std::string text = "";
		std::string oldText = "";

		glm::vec3 color = glm::vec3(1, 1, 1);
		glm::vec2 position = glm::vec2(0, 0);
		render::Font* font = nullptr;

		void updateBuffers();
		
		static render::Program* Program;
};
