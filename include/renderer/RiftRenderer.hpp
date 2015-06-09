#pragma once

#include "common.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include "GLUtil.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief RiftRenderer Renderer
 *
 * This renderer renders the scene to the Oculus Rift DK2.
 */
class RiftRenderer : public PerspectiveRenderer {
public:

	/**
	 * @brief Default constructor
	 *
	 * Parameters are passed to the PerspectiveRenderer class.
	 *
	 * @param fov Field of view
	 * @param aspectRatio Width / height
	 * @param zNear Near z clipping plane
	 * @param zFar Far z clipping  plane
	 */
	RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar);

	/**
	 * @brief Default destructor
	 */
	virtual ~RiftRenderer () = default;

	/**
	 * @brief Clears the buffers and background
	 *
	 * @param background Background color
	 */
	virtual void clear (Vector3f background);

	/**
	 * @brief Updates the state
	 *
	 * This method is always called before Renderer::draw();
	 */
	virtual void update (Matrix4f &s, Matrix4f &r, Matrix4f &t);

	/**
	 * @brief Draws the loaded data
	 *
	 * This method is always called after Renderer::update();
	 */
	virtual void draw ();

	/**
	 * @brief Do some pre processing
	 */
	virtual void preProcess ();

	/**
	 * @brief To the necessary clean up
	 */
	virtual void cleanUp ();

	/**
	 * @return Brief info about the renderer
	 */
	virtual const std::string info () const {
		return tfm::format(
			"RiftRenderer[\n"
			"  FOV = %d°,\n"
			"  aspectRatio = %d,\n"
			"  zNear = %d,\n"
			"  zFar = %d,\n"
			"  Frustum Width = %d,\n"
			"  Frustum Height = %d,\n"
			"  OVR[\n"
			"    Type = %s\n"
			"  ]\n"
			"]",
			fov,
			aspectRatio,
			zNear, zFar,
			fH, fW,
			hmd->ProductName
		);
	}

	/**
	 * @brief Returns the class type
	 */
	virtual const RendererType getClassType () const {
		return EHMDRenderer;
	}

protected:

	/**
	* @brief Uploads a DNC quad in the back of the scene to draw the leap images on
	*/
	virtual void uploadBackgroundCube();

	/**
	* @brief Uploads the leap textures to the shader and draws them in the background of the scene
	*/
	virtual void drawOnCube(ovrEyeType eye);

protected:

	GLFramebuffer frameBuffer[2]; ///< The framebuffer which we draw to with the rift for the left and right eye
	ovrEyeRenderDesc eyeRenderDesc[2]; ///< Render structure
	ovrGLConfig cfg; ///< Oculus config
	ovrGLTexture eyeTexture[2]; ///< OVR textures for distortion rendering

	/**
	* Leap passthrough
	*/
	GLuint leapVAO, leapV_VBO, leapUV_VBO, leapF_VBO; ///< Background cube
	GLuint leap_PBO[2][2][2]; ///< Pixel buffer objects
	std::shared_ptr<GLShader> leapShader; ///< Leap passthrough shader
	GLuint leapRawTexture[2]; ///< Distorted passthrough textures
	GLuint leapDistortionTexture[2];///< Passthrough textures

	int rawWidth = 640, rawHeight = 240; ///< Leap texture sizes
	int distWidth = 64, distHeight = 64; ///< Leap texture sizes
};

VR_NAMESPACE_END
