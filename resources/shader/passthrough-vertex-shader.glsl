#version 330

uniform mat4 mvp;

in vec3 position;
in vec2 texCoord;

out vec2 uv;

void main () {
	uv = texCoord;
	gl_Position = mvp * vec4(position, 1.0);
}