#version 330 compatibility

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec2 vOffset;

out vec2 uv;

void main() {
	gl_Position = vec4(vPosition + vOffset, 0.0, 1.0);
	uv = vUV;
}
