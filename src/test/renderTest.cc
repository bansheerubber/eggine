#include <glad/gl.h>
#include "renderTest.h"

#include <stdio.h>

#include "renderTestContainer.h"

RenderTest::RenderTest(RenderTestContainer* container) {
	container->addRenderObject(this);
}

void RenderTest::updateDisplayList(RenderTestContainer* container) {

}

void RenderTest::render(double deltaTime, RenderContext &context) {
	// this->offset[0] += this->direction[0] * deltaTime / 10.f;
	// this->offset[1] += this->direction[1] * deltaTime / 10.f;

	// if(this->offset[0] > 1) {
	// 	this->direction[0] = -1;
	// }
	// else if(this->offset[0] < -1) {
	// 	this->direction[0] = 1;
	// }

	// if(this->offset[1] > 1) {
	// 	this->direction[1] = -1;
	// }
	// else if(this->offset[1] < -1) {
	// 	this->direction[1] = 1;
	// }

	// glUniform2fv(RenderTest::uniforms[0], 1, &this->offset[0]);
	// glDrawArrays(GL_TRIANGLES, 0, 3);
}
