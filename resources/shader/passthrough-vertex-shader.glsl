#version 330

in vec3 position;
in vec2 texCoord;

out vec2 vTexCoord;

void main () {
	vTexCoord = texCoord;

	// We don't need to project, we're already in NDCs!
	gl_Position = vec4(position, 1.0f);
}