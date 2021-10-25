#version 430 core

layout(location = 0) in vec2 vVertex;
layout(location = 1) in vec2 vUV;
out vec2 uv;

layout(binding = 0) uniform vertexBlock
{
	mat4 projection;
} vb;

void main() {
	gl_Position = vb.projection * vec4(vVertex, 0.0, 1.0);
	uv = vUV;
}
