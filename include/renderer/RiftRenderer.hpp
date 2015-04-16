#pragma once

#include "common.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "OVR_Math.h"

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
	virtual ~RiftRenderer ();

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
	virtual void update ();

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

//	GLFWMonitor *getHmdDisplay ();

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
	const RendererType getClassType () const {
		return EHMDRenderer;
	}

protected:
	ovrHmd hmd; ///< Head mounted device
	GLFramebuffer frameBuffer; ///< The framebuffer which we draw to with the rift
	ovrGLTexture eyeTexture[2];
	ovrRecti eyeRenderViewport[2]; ///< Viewport for left and right eye
	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrGLConfig cfg; ///< Oculus config
	OVR::Sizei renderTargetSize; ///< Render buffer/texture size
};

VR_NAMESPACE_END
