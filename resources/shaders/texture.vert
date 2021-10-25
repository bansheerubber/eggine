#version 430 core

layout(location = 0) in vec2 vVertex;
layout(location = 1) in vec2 vUV;
out vec2 uv;

void main() {
	gl_Position = vec4(vVertex, 0.0, 1.0);
	uv = vUV;
}
