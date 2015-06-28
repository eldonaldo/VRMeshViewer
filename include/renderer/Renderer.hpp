#pragma once

#include "common.hpp"
#include "GLUtil.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Pin.hpp"
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
		, projectionMatrix(Matrix4f::Identity()), hmd(nullptr), showHands(true), sphereRadius(0.f) {

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
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	/**
	 * @brief Allows the renderer to do some processing before the render loop is entered.
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
	* @return View Matrix Leap
	*/
	const Matrix4f &getLeapViewMatrix() const {
		return leapViewMatrix;
	}

	/**
	* @param viewMatrix View Matrix Leap
	*/
	void setViewMatrixLeap(const Matrix4f &viewMatrix) {
		this->leapViewMatrix = viewMatrix;
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
	 * @brief Handle to the GFLW viewerGLFWwindow for rendering
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

	/**
	* @brief Sets the sphere radius
	*/
	void setSphereRadius(float r) {
		sphereRadius = r;
	}

	/**
	* @brief Sets the sphere center
	*/
	void setSphereCenter(Vector3f &c) {
		sphereCenter = c;
	}

	/**
	* @brief Sets leap frame
	*/
	void setFrame(Leap::Frame &f) {
		frame = f;
	}

	/*
	* @brief Upload a pin to the graphics card
	*/
	void uploadAnnotation(std::shared_ptr<Pin> &p, std::vector<std::shared_ptr<Pin>> &pl) {
		p->upload(shader);
		pinList = &pl;
	}

protected:

	std::shared_ptr<GLShader> shader; ///< Bounded shader
	std::shared_ptr<Mesh> mesh; ///< Bounded mesh
	Cube bbox; ///< Visual representation of the bounding box of the mesh
	std::shared_ptr<SkeletonHand> leftHand, rightHand; ///< Leap hands
	float FBWidth, FBHeight; ///< To avoid cyclic includes and incomplete type errors
	GLFWwindow *window; ///< GFLW viewerGLFWwindow handle
	ovrHmd hmd; ///< Head mounted device
	bool showHands; ///< Display leap hands
	Leap::Controller leapController; ///< Leap controller
	Sphere sphere; ///< Bounding hand sphere
	Vector3f sphereCenter; ///< Sphere center
	float sphereRadius; ///< Sphere radius
	Leap::Frame frame; ///< Leap motion frame
	std::vector<std::shared_ptr<Pin>> *pinList = nullptr; ///< List of pins

private:

	Matrix4f viewMatrix; ///< View matrix
	Matrix4f leapViewMatrix; ///< View for Leap camera
	Matrix4f projectionMatrix; ///< Projection matrix
};

VR_NAMESPACE_END
