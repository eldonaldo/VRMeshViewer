#version 330

// Point light representation
struct Light {
	vec3 position;
	vec3 intensity;
};

uniform Light light;
uniform float intensity;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

uniform float alpha;
uniform int wireframe = 0;

in vec3 vertexNormal;
in vec3 vertexPosition;

out vec4 color;

void main () {
	// Without wireframe overlay
	if (wireframe == 0) {
		// Transform normal
		vec3 normal = normalize(normalMatrix * vertexNormal);
		
		// Position of fragment in world coodinates
		vec3 position = vec3(modelMatrix * vec4(vertexPosition, 1.0));
		
		// Calculate the vector from surface to the light
		vec3 surfaceToLight = light.position - position;
		
		// Calculate the cosine of the angle of incidence = brightness
		float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
		brightness = clamp(brightness, 0.0, 1.0);
		
		// Calculate final color of the pixel
		color = vec4(vec3(intensity) * brightness * light.intensity, alpha);
	} else {
		// Draw wireframe overlay with solid lines
		color = vec4(0.2, 0.2, 0.2, 1.0);
	}
}