#version 330 compatibility

layout (location = 0) in vec2 vVertex;
layout (location = 1) in vec2 vUV;
out vec2 uv;

uniform mat4 projection;

void main() {
	gl_Position = projection * vec4(vVertex, 0.0, 1.0);
	uv = vUV;
}
