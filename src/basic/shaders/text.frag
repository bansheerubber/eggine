#version 330 compatibility

layout(location = 0) out vec4 color;

in vec2 uv;

uniform sampler2D textTexture;
uniform vec3 textColor;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textTexture, uv).r);
	color = vec4(textColor, 1.0) * sampled;
}
