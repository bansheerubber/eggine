#version 430 core

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec2 vOffset;
layout(location = 3) in int vTextureIndex;
layout(location = 4) in vec4 vColor;

layout(std140, binding = 0) uniform vertexBlock
{
	mat4 projection;
	vec2 chunkScreenSpace;
	float spritesheetWidth;
	float spritesheetHeight;
	float spriteWidth;
	float spriteHeight;
	float spritesOnRow;
} vb;

out vec2 uv;
out vec4 tileColor;

void main() {
	gl_Position = vb.projection * vec4(vPosition + vOffset + vb.chunkScreenSpace, 0.0f, 1.0f);

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
}
