#version 430 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;

layout(binding = 1) uniform sampler2D inTexture;

void main() {
	color = texture(inTexture, uv);
	// color = vec4(1, 0, 0, 1);
}
