#version 430 core

layout(location = 0) out vec4 color;

in vec2 uv;

layout(binding = 0) uniform sampler2D spriteTexture;
layout(std140, binding = 1) uniform fragmentBlock
{
	vec4 color;
} fb;

void main() {
	color = texture(spriteTexture, uv) * fb.color;
}
