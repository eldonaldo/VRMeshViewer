#pragma once

#include "common.hpp"
#include "GLUtil.hpp"
#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

/**
 *
 */
class Renderer {
public:
	/**
	 * @brief Default constructor
	 */
	Renderer (std::shared_ptr<GLShader> &s)
		: shader(s) {

	};

	/**
	 * @brief Default destructor
	 */
	virtual ~Renderer () {
		shader->free();
	}

	/**
	 * @brief Updates the state
	 *
	 * This method must be implemented by all subclasses. This method is
	 * always called before Renderer::draw();
	 */
	virtual void update () = 0;

	/**
	 * @brief Draws the loaded data
	 *
	 * This method must be implemented by all subclasses. This method is
	 * always called after Renderer::update();
	 */
	virtual void draw () = 0;

	/**
	 * @brief Allows the renderer to do some processing
	 * 		  before the render loop is entered.
	 *
	 * The default implementation does nothing.
	 */
	virtual void preProcess () { };

	/**
	 * @brief Sets the mesh
	 *
	 * @param m Mesh
	 */
	void setMesh (std::shared_ptr<Mesh> &m) {
		mesh = m;
	};

	/**
	 * @brief To the necessary clean up
	 */
	virtual void cleanUp () = 0;

	/**
	 * @return Mode Matrix
	 */
	const Matrix4f& getModelMatrix () const {
		return modelMatrix;
	}

	/**
	 * @param modelMatrix Model Matrix
	 */
	void setModelMatrix (const Matrix4f& modelMatrix) {
		this->modelMatrix = modelMatrix;
	}

	/**
	 * @return MVP Matrix
	 */
	const Matrix4f& getMvp () const {
		return mvp;
	}

	/**
	 * @return Projection Matrix
	 */
	const Matrix4f& getProjectionMatrix () const {
		return projectionMatrix;
	}

	/**
	 * @param projectionMatrix Projection Matrix
	 */
	void setProjectionMatrix (const Matrix4f& projectionMatrix) {
		this->projectionMatrix = projectionMatrix;
	}

	/**
	 * @return View Matrix
	 */
	const Matrix4f& getViewMatrix () const {
		return viewMatrix;
	}

	/**
	 * @param viewMatrix View Matrix
	 */
	void setViewMatrix (const Matrix4f& viewMatrix) {
		this->viewMatrix = viewMatrix;
	}

protected:

	std::shared_ptr<Mesh> mesh; ///< Bounded mesh
	std::shared_ptr<GLShader> shader; ///< Bounded shader
	Matrix4f viewMatrix; ///< View matrix
	Matrix4f modelMatrix; ///< Model matrix
	Matrix4f projectionMatrix; ///< Projection matrix
	Matrix4f mvp; ///< MVP matrix
};

VR_NAMESPACE_END
