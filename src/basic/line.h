#pragma once

#include <glm/vec2.hpp>
#include <vector>

#include "../renderer/program.h"
#include "renderObject.h"
#include "../renderer/vertexAttributes.h"
#include "../renderer/vertexBuffer.h"

using namespace std;

class Line: public RenderObject {
	public:
		Line();
		~Line();
		
		void addPosition(glm::vec2 position);
		void commit();

		void render(double deltaTime, RenderContext &context);
	
	private:
		static render::Program* Program;
		
		render::VertexBuffer* buffer;
		render::VertexAttributes* attributes;

		vector<glm::vec2> positions;

		glm::vec2* positionBuffer = nullptr;
};
