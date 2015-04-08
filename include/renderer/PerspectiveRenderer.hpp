#pragma once

#include "common.hpp"
#include "renderer/Renderer.hpp"

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
	PerspectiveRenderer (float fov, float aspectRatio, float zNear, float zFar);

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

private:
	float fov; ///> Field of view
	float aspectRatio; ///< Width / height
	float zNear, zFar; ///< Clipping planes
};

VR_NAMESPACE_END
