#pragma once

#include "common.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "OVR_Math.h"
#include "OVR_CAPI_GL.h"

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

	GLFramebuffer frameBuffer[2]; ///< The framebuffer which we draw to with the rift for the left and right eye
	ovrEyeRenderDesc eyeRenderDesc[2]; ///< Render structure
	ovrGLConfig cfg; ///< Oculus config
	float yaw; ///< Heads yaw angle
};

VR_NAMESPACE_END
