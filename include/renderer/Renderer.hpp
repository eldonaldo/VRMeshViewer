#pragma once

#include "common.hpp"
#include "GLSLProgram.hpp"

VR_NAMESPACE_BEGIN

/**
 *
 */
class Renderer {
public:
	Renderer ();
	virtual ~Renderer ();

	/**
	 * @brief Updates the state
	 *
	 * This method must be implemented by all subclasses. This method is
	 * always called before Renderer::draw();
	 *
	 * NOTE:
	 *
	 * Besides it calculates the MVP matrix and the inverse transpose of
	 * the model matrix and the inverse of the view matrix. These matrices
	 * are sometimes handy to have.
	 *
	 * So MAKE SURE that the appropriate model, view and projection matrices
	 * are calculated in advance (maybe in the constructor of the subclass) and
	 * that you call Renderer::update() in the derived update() method.
	 */
	virtual void update () {
//		// Calculate model-view-projection matrix
//		mvp = projectionMatrix * viewMatrix * modelMatrix;
//
//		// Inverse transpose model matrix
//		modelMatrixInverseTranspose = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
//
//		// Inverse view matrix
//		viewMatrixInverse = glm::inverse(glm::mat3(viewMatrix));
	}

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
	virtual void preProcess () {};

	/**
	 * @brief To the necessary clean up
	 */
	virtual void cleanUp () = 0;

	/**
	 * @brief Assign the currently used shader
	 * @param s Shared Pointer to the shader resource
	 */
	void setShader (std::shared_ptr<GLSLProgram> &s) {
		shader = s;
	}

protected:

	std::shared_ptr<GLSLProgram> shader; ///< Bounded shader
//	Matrix4f viewMatrix; ///< View matrix
//	Matrix3f viewMatrixInverse; ///< Inverse view matrix
//	Matrix4f modelMatrix; ///< Model matrix
//	Matrix3f modelMatrixInverseTranspose; ///< Inverse, transpose model matrix
//	Matrix4f projectionMatrix; ///< Projection matrix
//	Matrix4f mvp; ///< MVP matrix
};

VR_NAMESPACE_END
