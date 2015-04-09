#version 330

uniform mat4 mvp;
uniform float intensity;

out vec4 color;

void main (void) {
	color = vec4(vec3(intensity), 1.0);
}