#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Cube.hpp"
#include "mesh/Pin.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/RiftRenderer.hpp"
#include "GLUtil.hpp"
#include "leap/LeapListener.hpp"
#include "leap/SkeletonHand.hpp"
#include "leap/GestureHandler.hpp"
#include "Eigen/Geometry"

VR_NAMESPACE_BEGIN

/**
 * @brief Basis mesh viewer class.
 *
 * This class uses GLFW to construct a OS specific
 * window and allows all modern extension to be loaded with GLEW on
 * windows machines.
 */
class Viewer {
public:

	/**
	 * @brief Default constructor
	 */
	Viewer (const std::string &title, int width, int height, bool useRift = false, bool debug = false) throw ();

	/**
	 * @brief Default constructor
	 */
	virtual ~Viewer ();

	/**
	 * @brief Attaches the Leap to the viewer
	 *
	 * @param l Leap Listener instance
	 */
	virtual void attachLeap (std::unique_ptr<LeapListener> &l);

	/**
	 * @brief Displays the data
	 *
	 * Internally it calls the Renderer::update() and Renderer::draw() methods. The purpose of
	 * Renderer::update() is that the states get updated and Renderer::draw() is responsible
	 * of drawing the data. Of course a renderer must be set in advance.
	 */
	virtual void display(std::shared_ptr<Mesh> &m, std::unique_ptr<Renderer> &r) throw ();

	/**
	* @brief Returns the arcball
	*/
	Arcball& getArcball ();

	/**
	* @brief Returns the model scale matrix
	*/
	Matrix4f& getScaleMatrix ();

	/**
	* @brief Returns the model translate matrix
	*/
	Matrix4f& getTranslateMatrix ();

	/**
	* @brief Returns the model rotation matrix
	*/
	Matrix4f& getRotationMatrix();

	/**
	* @brief Returns the last position used for the arcball
	*/
	Vector2i& getLastPos();

	/**
	* @brief Set the last position for the arcball
	*/
	void setLastPos(Vector2i &v);

	/**
	 * @brief Retrieve some OpenGL infos
	 *
	 * @return Supported OpenGL Versions
	 */
	std::string info ();

	/**
	 * @brief Places the object in the world coordindate system and scales it for the immersion effect
	 */
	virtual void placeObject (std::shared_ptr<Mesh> &m);

	/**
	* @brief Returns the mesh
	*/
	std::shared_ptr<Mesh> getMesh ();

	/**
	* @brief Add an annotation
	*/
	void addAnnotation(Vector3f &pos, Vector3f &n);

protected:

	/**
	 * @brief Displays the FPS count in the title of the window and returns the calculated FPS
	 */
	virtual void calcAndAppendFPS ();

public:

	int FBWidth, FBHeight; ///< Framebuffer size
	int width, height; ///< Window width and height
	Vector3f sphereCenter; ///< Sphere center
	float sphereRadius; ///< Sphere radius
	Vector3f annotationTarget; ///< Annotation target
	Vector3f annotationNormal; ///< Annotation normal
	bool uploadAnnotation; ///< flag whether to upload a annotation or not

protected:

	ovrHmd hmd; ///< Head mounted device
	GLFWwindow *window; ///< GLFW window pointer
	const std::string title; ///< Window title
	Color3f background; ///< Background color
	const float interval; ///< Interval to refresh FPS in seconds
	unsigned int frameCount = 0; ///< Frame count
	double fps = 0.0; ///< FPS count
	bool appFPS = true; ///< If true, then the current FPS count is appended to the window title
	bool useRift; ///< HMD mode
	std::unique_ptr<Renderer> renderer; ///< Bounded renderer
	std::shared_ptr<Mesh> mesh; ///< Pointer to mesh
	Arcball arcball; ///< Arcball
	Matrix4f scaleMatrix; ///< Scale matrix
	Matrix4f rotationMatrix; ///< Rotation matrix
	Matrix4f translateMatrix; ///< Translation matrix
	Vector2i lastPos; ///< Last click position used for the arcball
	bool debug; ///< Debug mode
	Leap::Controller leapController; ///< Leap controller
	std::unique_ptr<LeapListener> leapListener; ///< Leap listener instance
	std::shared_ptr<SkeletonHand> hands[2]; ///< Leap hands
	std::shared_ptr<GestureHandler> gestureHandler; ///< Gesture handler
	Leap::Frame frame; ///< Leap Frame
	std::vector<std::shared_ptr<Pin>> pinList; ///< List of annotations
};

VR_NAMESPACE_END
