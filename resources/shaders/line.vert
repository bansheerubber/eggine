#version 430 core

layout(location = 0) in vec2 vertex;

layout(binding = 0) uniform vertexBlock
{
	mat4 projection;
} vb;

void main() {
	gl_Position = vb.projection * vec4(vertex, 0.0, 1.0);
}
