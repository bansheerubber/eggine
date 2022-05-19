#version 430 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;

layout(binding = 1) uniform fragmentBlock
{
	vec3 textColor;
} fb;
layout(binding = 2) uniform sampler2D textTexture;

void main() {
	color = vec4(fb.textColor, texture(textTexture, uv).r);
}
