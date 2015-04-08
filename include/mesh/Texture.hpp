#pragma once

#include "common.hpp"
#include "GL/glew.h"

VR_NAMESPACE_BEGIN

/**
 * @brief Representation of an OpenGL texture
 */
class Texture {
public:

	/**
	 * @brief Construct a texture form a path
	 *
	 * @param textureTarget OpenGL texture target
	 * @param path Filename
	 */
	Texture (GLenum textureTarget, const std::string &path);

	/**
	 * @brief Construct a texture from a byte array
	 *
	 * @param textureTarget OpenGL texture target
	 * @param path Filename
	 */
	Texture (GLenum textureTarget, unsigned char *bytes, int width, int height);

	/**
	 * @brief Default destructor
	 */
	virtual ~Texture ();

	/**
	 * @brief Load texture
	 */
	void load () throw ();

	/**
	 * @brief Binds the texture into the OpenGL context
	 *
	 * @param textureUnit Texture unit
	 */
	void bind (GLenum textureUnit);

private:

	/**
	 * @brief Invalid texture marker
	 */
	static const GLuint INVALID_TEXTURE = -1;

	/**
	 * @brief Allocates OpenGL Buffers and copies the data
	 *
	 * @param data Image data in bytes
	 * @param width Image width
	 * @param height Image height
	 */
	void constructBuffers (unsigned char *data, int width, int height);

    GLenum textureTarget; ///< Texture type
    std::string path; ///< Filename
    GLuint textureObj; ///< OpenGL ID
    std::unique_ptr<unsigned char> image; ///< Image data in bytes
};

VR_NAMESPACE_END
