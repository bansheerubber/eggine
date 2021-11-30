#version 430 core

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform fragmentBlock
{
	vec4 color;
} fb;

void main() {
	color = fb.color;
}
