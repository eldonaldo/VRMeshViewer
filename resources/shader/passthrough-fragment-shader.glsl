#version 330

uniform sampler2D rawTexture;
uniform sampler2D distortionTexture;

in  vec2 vTexCoord;
out vec4 color;

void main () {
	vec2 distortionIndex = texture2D(distortionTexture, vTexCoord).xy;
	float hIndex = distortionIndex.r;
	float vIndex = distortionIndex.g;

	if(vIndex > 0.0 && vIndex < 1.0 && hIndex > 0.0 && hIndex < 1.0) {
		color = vec4(texture2D(rawTexture, distortionIndex).rrr, 1.0);
	} else {
		color = vec4(0.8, 0.0, 0.0, 1.0);
	}
}