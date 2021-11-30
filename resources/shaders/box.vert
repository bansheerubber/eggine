#version 430 core

layout(location = 0) in vec2 vVertex;

layout(std140, binding = 0) uniform vertexBlock
{
	mat4 projection;
	vec2 position;
	vec2 size;
} vb;

void main() {
	gl_Position = vb.projection * vec4((vVertex * vb.size) + vb.position, 0.0, 1.0);
}
