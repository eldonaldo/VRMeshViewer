#include "mesh/Texture.hpp"

// Need to include it here
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VR_NAMESPACE_BEGIN

Texture::Texture (GLenum textureTarget, const std::string &path)
	: textureTarget(textureTarget), path(path), textureObj(INVALID_TEXTURE) {

}

Texture::Texture (GLenum textureTarget, unsigned char *bytes, int width, int height) {
	constructBuffers(bytes, width, height);
}

void Texture::bind (GLenum textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(textureTarget, textureObj);
}

void Texture::load () throw () {
	int width, height, comp;
	image = std::unique_ptr<unsigned char>(stbi_load(path.c_str(), &width, &height, &comp, STBI_rgb_alpha));

	if (!image)
		throw VRException("Unable to load image: %s", path);

	constructBuffers(image.get(), width, height);
}

void Texture::constructBuffers (unsigned char *data, int width, int height) {
	// Transfer image to GPU memory
	glGenTextures(1, &textureObj);
	glBindTexture(textureTarget, textureObj);
	glTexImage2D(textureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(textureTarget, 0);
}

Texture::~Texture () {
	if (textureObj != INVALID_TEXTURE)
		glDeleteTextures(1, &textureObj);
}

VR_NAMESPACE_END
