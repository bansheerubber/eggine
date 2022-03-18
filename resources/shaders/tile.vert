#version 430 core

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vOffset;
layout(location = 3) in int vTextureIndex;
layout(location = 4) in vec4 vColor;
layout(location = 5) in int vIsXray;

layout(std140, binding = 0) uniform vertexBlock
{
	mat4 projection;
	vec3 chunkScreenSpace;
	float spritesheetWidth;
	float spritesheetHeight;
	float spriteWidth;
	float spriteHeight;
	float spritesOnRow;
	int timer;
} vb;

out vec2 uv;
out vec4 tileColor;
flat out int xray;
out float height;
flat out int timer;

void main() {
	gl_Position = vb.projection * vec4(vPosition + vOffset.xy + vb.chunkScreenSpace.xy, (vOffset.z + vb.chunkScreenSpace.z) / 100.0f, 1.0f);

	float x = floor(mod(float(vTextureIndex), vb.spritesOnRow));
	float y = floor(float(vTextureIndex) / vb.spritesOnRow);

	vec2 minUV = vec2(
		(vb.spriteWidth * x + 2.0 * x + 1.0) / vb.spritesheetWidth,
		(vb.spriteHeight * y + 2.0 * y + 1.0) / vb.spritesheetHeight
	);

	vec2 maxUV = vec2(
		(vb.spriteWidth * x + 2.0 * x + 1.0 + vb.spriteWidth) / vb.spritesheetWidth,
		(vb.spriteHeight * y + 2.0 * y + 1.0 + vb.spriteHeight) / vb.spritesheetHeight
	);

	uv = mix(minUV, maxUV, vUV);
	tileColor = vColor;
	xray = vIsXray;
	height = vb.spriteHeight;
	timer = vb.timer;
}
