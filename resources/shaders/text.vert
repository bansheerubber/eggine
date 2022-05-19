#version 430 core

layout(location = 0) in vec2 vVertex;
layout(location = 1) in vec2 vUV;

layout(location = 0) out vec2 uv;

layout(std140, binding = 0) uniform vertexBlock
{
	mat4 projection;
	vec2 position;
} vb;

void main() {
	gl_Position = vb.projection * vec4(vVertex + vb.position, 0.0, 1.0);
	uv = vUV;
}
