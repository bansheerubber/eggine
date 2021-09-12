#version 330 compatibility

layout(location = 0) out vec4 color;

in vec2 uv;

uniform sampler2D spriteTexture;

void main() {
	color = texture(spriteTexture, uv);
}
