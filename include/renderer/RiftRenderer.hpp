#pragma once

#include "common.hpp"
#include "renderer/PerspectiveRenderer.hpp"

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
	RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float aspectRatio, float zNear, float zFar)
		: PerspectiveRenderer(shader, fov, aspectRatio, zNear, zFar) {

	}

	/**
	 * @brief Default destructor
	 */
	virtual ~RiftRenderer () = default;

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

	/**
	 * @return Brief info about the renderer
	 */
	virtual std::string info () {
		return tfm::format(
			"RiftRenderer[\n"
			"  FOV = %d°,\n"
			"  aspectRatio = %d,\n"
			"  zNear = %d,\n"
			"  zFar = %d,\n"
			"  Frustum Width = %d,\n"
			"  Frustum Height = %d,\n"
			"]",
			fov,
			aspectRatio,
			zNear, zFar,
			fH, fW
		);
	}

};

VR_NAMESPACE_END
