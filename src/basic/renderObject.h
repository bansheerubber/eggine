#pragma once

#include <GLFW/glfw3.h>

#include "gameObject.h"
#include "renderContext.h"

class RenderObject : public GameObject {
	public:
		RenderObject();
		RenderObject(bool addToList) {};
		
		virtual void render(double deltaTime, RenderContext &context) = 0;

		bool visible = true;
};
