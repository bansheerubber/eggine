#include <glad/gl.h>
#include "renderObject.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "../engine/engine.h"

RenderObject::RenderObject() {
	engine->addRenderObject(this);
}
