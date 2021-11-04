#version 430 core

layout(location = 0) out vec4 color;

in vec2 uv;
in vec4 tileColor;

layout(binding = 0) uniform sampler2D spriteTexture;

void main() {
	color = texture(spriteTexture, uv) * tileColor;
}
