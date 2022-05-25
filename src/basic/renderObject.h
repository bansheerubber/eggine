#pragma once

#include "gameObject.h"
#include "renderContext.h"

class RenderObject : public GameObject {
	public:
		RenderObject();
		RenderObject(bool addToList) {};
		virtual ~RenderObject();
		
		virtual void render(double deltaTime, RenderContext &context) = 0;

	private:
		bool visible = true;
};
