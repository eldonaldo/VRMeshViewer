#pragma once

#include "common.hpp"
#include "GLUtil.hpp"
#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

/**
 * Forward declaration
 */
class Viewer;

/**
 * @brief Available renderer types
 */
enum RendererType {
	ENormalRenderer,
	EHMDRenderer
};

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
	 * @brief Clears the buffers and background
	 *
	 * @param background Background color
	 */
	virtual void clear (Vector3f background) {
		glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	 * @return Brief info about the renderer
	 */
	virtual const std::string info () const = 0;

	/**
	 * @brief Returns the class type
	 */
	const RendererType getClassType () const {
		return ENormalRenderer;
	}

	/**
	 * @return Model Matrix
	 */
	const Matrix4f &getModelMatrix () const {
		return modelMatrix;
	}

	/**
	 * @return Transpose inverse model matrix
	 */
	const Matrix3f &getNormalMatrix () const {
		return normalMatrix;
	}

	/**
	 * @param modelMatrix Model Matrix
	 */
	void setModelMatrix (const Matrix4f &modelMatrix) {
		this->modelMatrix = modelMatrix;
		mvp = projectionMatrix * viewMatrix * modelMatrix;

		// Calculate normal matrix for normal transformation
		Matrix3f tmp = modelMatrix.topLeftCorner<3, 3>();
		Matrix3f inv = tmp.inverse();
		normalMatrix = inv.transpose();
	}

	/**
	 * @return MVP Matrix
	 */
	const Matrix4f &getMvp () const {
		return mvp;
	}

	/**
	 * @return Projection Matrix
	 */
	const Matrix4f &getProjectionMatrix () const {
		return projectionMatrix;
	}

	/**
	 * @param projectionMatrix Projection Matrix
	 */
	void setProjectionMatrix (const Matrix4f &projectionMatrix) {
		this->projectionMatrix = projectionMatrix;
		mvp = projectionMatrix * viewMatrix * modelMatrix;
	}

	/**
	 * @return View Matrix
	 */
	const Matrix4f &getViewMatrix () const {
		return viewMatrix;
	}

	/**
	 * @param viewMatrix View Matrix
	 */
	void setViewMatrix (const Matrix4f &viewMatrix) {
		this->viewMatrix = viewMatrix;
		mvp = projectionMatrix * viewMatrix * modelMatrix;
	}

	/**
	 * @brief Updates the frame buffer size
	 * @param w Width
	 * @param h Height
	 */
	void updateFBSize (float w, float h) {
		FBWidth = w;
		FBHeight = h;
	}

	/**
	 * @brief Handle to the GFLW window for rendering
	 */
	void setWindow (GLFWwindow *w) {
		window = w;
	}

protected:

	std::shared_ptr<Mesh> mesh; ///< Bounded mesh
	std::shared_ptr<GLShader> shader; ///< Bounded shader
	float FBWidth, FBHeight; ///< To avoid cyclic includes and incomplete type errors
	GLFWwindow *window; ///< GFLW window handle

private:

	Matrix4f viewMatrix; ///< View matrix
	Matrix4f modelMatrix; ///< Model matrix
	Matrix3f normalMatrix; ///< Transpose inverse model matrix
	Matrix4f projectionMatrix; ///< Projection matrix
	Matrix4f mvp; ///< MVP matrix
};

VR_NAMESPACE_END
