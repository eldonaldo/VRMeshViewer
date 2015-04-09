#version 330

uniform mat4 mvp;
uniform float intensity;

in vec3 position;

void main (void) {
	gl_Position = mvp * vec4(position, 1.0);
}