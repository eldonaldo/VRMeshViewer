#version 330

uniform sampler2D rawTexture;
uniform sampler2D distortionTexture;

in vec2 uv;
out vec4 color;

void main () {
	vec4 index = texture(distortionTexture, uv);

	// Only use xy within [0, 1]
	if (index.r > 0.0 && index.r < 1.0 && index.g > 0.0 && index.g < 1.0)
    	color = vec4(texture(rawTexture, index.rg).rrr, 1);
	else
    	color = vec4(0, 0, 0, 1);
}