#version 430 core

layout(location = 0) out vec4 color;

in vec2 uv;
flat in int textureIndex;

uniform sampler2D spriteTexture;

void main() {
	float width = 1057.0;
	float height = 391.0;
	float spriteWidth = 64.0;
	float spriteHeight = 128.0;
	float padding = 1;
	int spritesOnRow = int(width) / int(spriteWidth);
	int x = textureIndex % spritesOnRow;
	int y = textureIndex / spritesOnRow;

	vec2 minUV = vec2(
		(spriteWidth * float(x) + 2.0 * float(x) + 1.0) / width,
		(spriteHeight * float(y) + 2.0 * float(y) + 1.0) / height
	);

	vec2 maxUV = vec2(
		(spriteWidth * float(x) + 2.0 * float(x) + 1.0 + spriteWidth) / width,
		(spriteHeight * float(y) + 2.0 * float(y) + 1.0 + spriteHeight) / height
	);
	
	color = texture(spriteTexture, mix(minUV, maxUV, uv));
}
