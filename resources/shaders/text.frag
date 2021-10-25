#version 430 core

layout(location = 0) out vec4 color;

in vec2 uv;

layout(binding = 0) uniform sampler2D textTexture;
layout(binding = 1) uniform fragmentBlock
{
	vec3 textColor;
} fb;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textTexture, uv).r);
	color = vec4(fb.textColor, 1.0) * sampled;
}
