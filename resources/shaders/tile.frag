#version 430 core

layout(location = 0) out vec4 color;

in vec2 uv;
in vec4 tileColor;
flat in int occluded;
in float height;
flat in int timer;

layout(binding = 0) uniform sampler2D spriteTexture;

void main() {
	color = texture(spriteTexture, uv);
	if(color.a < 0.1) {
		discard;
	}
	
	if(occluded == 1) {
		if(mod((int(uv.y * height * 10000) - timer * 500) / 20000, 2) == 0) {
			color = (color * tileColor * vec4(0.7, 0.7, 0.7, 0.5) + vec4(0.0, 0.1, 0.2, 0.0)) * vec4(1.3, 1.3, 1.3, 1.0);
		}
		else {
			color = (color * tileColor * vec4(0.7, 0.7, 0.7, 0.9) + vec4(0.0, 0.1, 0.2, 0.0)) * vec4(1.3, 1.3, 1.3, 1.0);
		}
	}
	else {
		color = color * tileColor;
	}

	// color = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}
