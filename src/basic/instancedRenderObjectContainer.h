#pragma once

#include <vector>

#include "renderContext.h"
#include "renderObject.h"

using namespace std;

template<class T>
class InstancedRenderObjectContainer : public RenderObject {
	public:
		void addRenderObject(T* object) {
			this->objects.push_back(object);
		}

		void removeRenderObject(T* object) {
			auto it = find(this->objects.begin(), this->objects.end(), object);
			if(it != this->objects.end()) {
				this->objects.erase(it);
			}
		}

		virtual void render(double deltaTime, RenderContext &context) = 0;
	
	protected:
		vector<T*> objects;
};
