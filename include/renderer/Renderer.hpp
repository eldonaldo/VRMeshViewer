#pragma once

#include "common.hpp"
#include "GLUtil.hpp"
#include "mesh/Mesh.hpp"
#include "Leap.h"
#include "leap/SkeletonHand.hpp"

VR_NAMESPACE_BEGIN

/// Forward declaration
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
		: shader(s), FBWidth(0), FBHeight(0), window(nullptr), viewMatrix(Matrix4f::Identity())
		, projectionMatrix(Matrix4f::Identity()), hmd(nullptr), showHands(true) {

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
	 * @brief Allows the renderer to do some processing
	 * 		  before the render loop is entered.
	 *
	 * The default implementation does nothing.
	 */
	virtual void preProcess () {}

	/**
	 * @brief Updates the state
	 *
	 * This method must be implemented by all subclasses. This method is
	 * always called before Renderer::draw();
	 */
	virtual void update (Matrix4f &s, Matrix4f &r, Matrix4f &t) = 0;

	/**
	 * @brief Draws the loaded data
	 *
	 * This method must be implemented by all subclasses. This method is
	 * called after Renderer::update();
	 */
	virtual void draw () = 0;

	/**
	 * @brief Sets the mesh
	 *
	 * @param m Mesh
	 */
	void setMesh (std::shared_ptr<Mesh> &m) {
		mesh = m;
	}

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
	virtual const RendererType getClassType () const {
		return ENormalRenderer;
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
	}

	/**
	* @return View Matrix Leap left
	*/
	const Matrix4f &getViewMatrixLeapLeft() const {
		return viewMatrixLeapLeft;
	}

	/**
	* @param viewMatrix View Matrix Leap left
	*/
	void setViewMatrixLeapLeft(const Matrix4f &viewMatrix) {
		this->viewMatrixLeapLeft = viewMatrix;
	}

	/**
	* @return View Matrix Leap right
	*/
	const Matrix4f &getViewMatrixLeapRight() const {
		return viewMatrixLeapRight;
	}

	/**
	* @param viewMatrix View Matrix Leap right
	*/
	void setViewMatrixLeapRight(const Matrix4f &viewMatrix) {
		this->viewMatrixLeapRight = viewMatrix;
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

	/**
	 * @brief Sets the pointer to the Hmd
	 */
	void setHmd (ovrHmd h) {
		hmd = h;
	}

	/**
	*@brief Set the leap hands
	*/
	void setHands(std::shared_ptr<SkeletonHand> &l, std::shared_ptr<SkeletonHand> &r) {
		leftHand = l;
		rightHand = r;
	}

	/**
	 * @brief Sets the Leap controller
	 */
	void setController(Leap::Controller &l) {
		leapController = l;
	}

protected:

	std::shared_ptr<Mesh> mesh; ///< Bounded mesh
	std::shared_ptr<GLShader> shader; ///< Bounded shader
	std::shared_ptr<SkeletonHand> leftHand, rightHand; ///< Leap hands
	std::unique_ptr<GLShader> bboxShader; ///< Bounding box shader
	float FBWidth, FBHeight; ///< To avoid cyclic includes and incomplete type errors
	GLFWwindow *window; ///< GFLW window handle
	ovrHmd hmd; ///< Head mounted device
	bool showHands; ///< Display leap hands
	Leap::Controller leapController; ///< Leap controller

private:

	Matrix4f viewMatrix; ///< View matrix
	Matrix4f viewMatrixLeapLeft; ///< View for Leap left camera
	Matrix4f viewMatrixLeapRight; ///< View for Leap right camera
	Matrix4f projectionMatrix; ///< Projection matrix
};

VR_NAMESPACE_END
