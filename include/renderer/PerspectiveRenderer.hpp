#pragma once

#include "common.hpp"
#include "renderer/Renderer.hpp"
#include "GLUtil.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief Perspective Renderer
 *
 * This renderer renders the scene in a normal
 * perspective camera.
 */
class PerspectiveRenderer : public Renderer {
public:
	/**
	 * @brief Default constructor
	 *
	 * @param fov Field of view
	 * @param aspectRatio Width / height
	 * @param zNear Near z clipping plane
	 * @param zFar Far z clipping  plane
	 */
	PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float aspectRatio, float zNear, float zFar);

	/**
	 * @brief Default destructor
	 */
	virtual ~PerspectiveRenderer () = default;

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
			"PerspectiveRenderer[\n"
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

private:
	float fov; ///> Field of view
	float aspectRatio; ///< Width / height
	float zNear, zFar; ///< Clipping planes
	float fH, fW; ///< Frustum width and height
};

VR_NAMESPACE_END
