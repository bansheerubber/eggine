#version 330 compatibility

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vOffset;

uniform mat4 projection;

out vec2 uv;

void main() {
	gl_Position = projection * vec4(vPosition + vOffset.xy, vOffset.z, 1.0f);
	uv = vUV;
}
