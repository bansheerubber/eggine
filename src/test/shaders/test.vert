#version 330 compatibility

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec2 vOffset;
layout (location = 3) in int vTextureIndex;

uniform mat4 projection;

out vec2 uv;
flat out int textureIndex;

void main() {
	gl_Position = projection * vec4(vPosition + vOffset, 0.0f, 1.0f);
	uv = vUV;
	textureIndex = vTextureIndex;
}
