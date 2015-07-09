#pragma once

#include "common.hpp"
#include "hdrloader.h"
#include "renderer/Renderer.hpp"
#include "mesh/Sphere.hpp"
#include "mesh/Environment.hpp"
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
	PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar);

	/**
	 * @brief Default destructor
	 */
	virtual ~PerspectiveRenderer () = default;

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
			"PerspectiveRenderer[\n"
			"  FOV = %d deg,\n"
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

	/**
	 * @brief Returns the class type
	 */
	virtual const RendererType getClassType () const {
		return ENormalRenderer;
	}

protected:

	/**
	* @brief Pre process global illumination
	*/
	void preProcessGI ();

protected:

	float fov; ///> Field of view
	float width, height; ///< Width and height
	float aspectRatio; ///< Width / height
	float zNear, zFar; ///< Clipping planes
	float fH, fW; ///< Frustum width and height
	Vector3f &materialColor; /// Material intensity
	Vector3f &cameraPosition; ///< Camera position
	Vector3f &lookAtPosition; ///< Look at position
	Vector3f &headsUp; ///< Camera heads up
	Vector3f &lightIntensity; ///< Light intensity
	Environment GISphere; /// Fake Global Illumination sphere
	std::shared_ptr<float> environment; /// Map
	std::shared_ptr<float> environmentDiffuse; /// Diffuse map
	GLuint envTexture; /// OpenGL Texture handles
	GLuint envDiffuseTexture; /// OpenGL Texture handles
};

VR_NAMESPACE_END
