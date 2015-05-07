#version 330

uniform mat4 mvp;

in vec3 position;
in vec3 normal;

out vec3 vertexNormal;
out vec3 vertexPosition;

void main () {
	// Pass through
	vertexNormal = normal;
	vertexPosition = position;
	
	gl_Position = mvp * vec4(position, 1.0);
}