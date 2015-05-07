#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 texCoord;

out vec2 vTexCoord;

void main () {
	vTexCoord = texCoord.xy;

	// We don't need to project, you're already in NDCs!
	gl_Position = position;
}