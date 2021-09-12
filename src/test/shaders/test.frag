#version 330 compatibility

layout(location = 0) out vec4 color;

in vec2 uv;

uniform sampler2D spriteTexture;

void main() {
	vec4 textureColor = texture(spriteTexture, uv);
	// color = vec4(vec3(gl_FragCoord.z), 1.0);
	color = textureColor;
}
