#version 430 core

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec2 vOffset;
layout(location = 3) in int vTextureIndex;

layout(binding = 0) uniform vertexBlock
{
	mat4 projection;
	vec2 chunkScreenSpace;
} vb;

out vec2 uv;
flat out int textureIndex;

void main() {
	gl_Position = vb.projection * vec4(vPosition + vOffset + vb.chunkScreenSpace, 0.0f, 1.0f);
	uv = vUV;
	textureIndex = vTextureIndex;
}
