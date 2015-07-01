#pragma once

#include "common.hpp"
#include "nanogui/screen.h"
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
#include "network/UDPSocket.hpp"
#include "nanogui/window.h"
#include "nanogui/layout.h"
#include "nanogui/label.h"
#include "nanogui/button.h"
#include "nanogui/checkbox.h"
#include "nanogui/textbox.h"
#include "nanogui/combobox.h"
#include "nanogui/messagedialog.h"
#include "nanogui/colorwheel.h"

VR_NAMESPACE_BEGIN

/**
 * @brief Basis mesh viewer class.
 *
 * This class uses GLFW to construct a OS specific
 * viewerGLFWwindow and allows all modern extension to be loaded with GLEW on
 * windows machines.
 */
class Viewer : public nanogui::Screen {
public:

	/**
	 * @brief Default constructor
	 */
	Viewer(const std::string &title, int width, int height, ovrHmd &hmd);

	/**
	 * @brief Default constructor
	 */
	virtual ~Viewer ();

	/**
	 * @brief Attaches the Leap to the viewer
	 *
	 * @param l Leap Listener instance
	 */
	void attachLeap (std::unique_ptr<LeapListener> &l);

	/**
	 * @brief Uploads and preprocesses the mesh
	 */
	void upload(std::shared_ptr<Mesh> &m);

	/**
	* @brief Add an annotation
	*/
	void addAnnotation(Vector3f &pos, Vector3f &n);

	/**
	* @brief Add an annotation with color
	*/
	void addAnnotation(const Vector3f &pos, const Vector3f &n, const Vector3f &c);

	/**
	* @brief Saves the annotations to specified file
	*/
	void saveAnnotations(const std::string &path);

	/**
	* @brief Saves the annotations to a generated file
	*/
	void saveAnnotations();

	/**
	* @brief Loads annotations from a file previous saved with saveAnnotations()
	*/
	void loadAnnotations(const std::string &s);

	/**
	 * @brief Sets the renderer
	 */
	void setRenderer (std::shared_ptr<Renderer> &r);

	/**
	* @brief Retrieve some OpenGL infos
	*
	* @return Supported OpenGL Versions
	*/
	std::string info();

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
	* @brief Returns the mesh
	*/
	std::shared_ptr<Mesh> &getMesh ();

	/**
	* @brief Returns the vector of annotations
	*/
	std::vector<std::shared_ptr<Pin>> &getAnnotations();

	/**
	 * @brief Returns a pointer to the renderer
	 */
	std::shared_ptr<Renderer> &getRenderer ();

	/**
	 * @brief Attaches a UDP socket
	 */
	void attachSocket (UDPSocket &s);

	/**
	* @brief Delete the pin if we hit it
	*/
	bool deletePinIfHit(Vector3f &position);

	/**
	 * @brief Render loop for 3D content with the rift
	 */
	void renderLoop ();

	/**
	 * @brief Displays the data
	 */
	void drawContents();

	/**
	 * @brief Mouse button events
	 */
	virtual bool mouseButtonEvent (const Vector2i &p, int button, bool down, int modifiers) override;

	/**
	 * @brief Mouse button events
	 */
	virtual bool mouseMotionEvent (const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;

	/**
	 * @brief Scroll events
	 */
	virtual bool scrollEvent (const Vector2i &p, const Vector2f &rel) override;

	/**
	 * @brief Mouse drag events
	 */
	virtual bool mouseDragEvent (const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;

	/**
	 * @brief Mouse enter events
	 */
	virtual bool mouseEnterEvent (const Vector2i &p, bool enter) override;

	/**
	 * @brief Frambuffer size changed
	 */
	virtual void framebufferSizeChanged () override;

	/**
	 * @brief Keyboard events
	 */
	bool keyboardEvent(int key, int scancode, bool action, int mods);

protected:

	/**
	 * @brief Places the object in the world coordindate system and scales it for the immersion effect and builds the kd-tree
	 */
	virtual void placeObjectAndBuildKDTree (std::shared_ptr<Mesh> &m);

	/**
	 * @brief Displays the FPS count in the title of the viewerGLFWwindow and returns the calculated FPS
	 */
	virtual void calcAndAppendFPS ();

	/**
	* @brief Serializes the annotation vector
	*/
	std::string serializeAnnotations(std::vector<std::shared_ptr<Pin>> &list);

	/**
	* @brief Serializes the annotation vector
	*/
	std::string serializeAnnotations(std::vector<Pin> &list);

	/**
	* @brief Loads annotations from a file. Called issued by method loadAnnotations()
	*/
	void loadAnnotationsDelayed ();

	/**
	* @brief Loads annotations from a string
	*/
	void loadAnnotationsFromString(std::string &s);

	/**
	* @brief Returns a list of pins serialized in the string s
	*/
	std::vector<Pin> getAnnotationsFromString(std::string &s);

	/**
	* @brief Checks if the pin is already contained in the pin list
	*/
	bool pinListContains(std::vector<std::shared_ptr<Pin>> &list, const Pin &p) const;

	/**
	 * @brief Serializes the translate, scale and rotation matrices
	 */
	std::string serializeTransformationState ();

	/**
	 * @brief Process networking operations
	 */
	void processNetworking ();

	/**
	 * @brief Place GUI objects
	 */
	void initGUI();

public:

	int FBWidth, FBHeight; ///< Framebuffer size
	int width, height; ///< Window width and height
	Vector3f sphereCenter; ///< Sphere center
	float sphereRadius; ///< Sphere radius
	Vector3f annotationTarget; ///< Annotation target
	Vector3f annotationNormal; ///< Annotation normal
	bool uploadAnnotation; ///< flag whether to upload a annotation or not

	Leap::Controller leapController; ///< Leap controller

protected:
	bool ready; ///< True when all geometry is uploaded to the GPU on the startup
	std::shared_ptr<Renderer> renderer; ///< Bounded renderer
	ovrHmd hmd; ///< Head mounted device
	GLFWwindow *viewerGLFWwindow; ///< GLFW viewerGLFWwindow pointer
	std::string title; ///< Window title
	Color3f background; ///< Background color
	const float interval; ///< Interval to refresh FPS in seconds
	unsigned int frameCount = 0; ///< Frame count
	double fps = 0.0; ///< FPS count
	bool appFPS = true; ///< If true, then the current FPS count is appended to the viewerGLFWwindow title
	std::shared_ptr<Mesh> mesh; ///< Pointer to mesh
	Arcball arcball; ///< Arcball
	Matrix4f scaleMatrix; ///< Scale matrix
	Matrix4f rotationMatrix; ///< Rotation matrix
	Matrix4f translateMatrix; ///< Translation matrix
	Vector2i lastPos; ///< Last click position used for the arcball
	std::unique_ptr<LeapListener> leapListener; ///< Leap listener instance
	std::shared_ptr<SkeletonHand> hands[2]; ///< Leap hands
	std::shared_ptr<GestureHandler> gestureHandler; ///< Gesture handler
	Leap::Frame frame; ///< Leap Frame
	std::vector<std::shared_ptr<Pin>> pinList; ///< List of annotations
	std::vector<Pin> pinListAdd; ///< List of annotations to aff for client
	std::vector<Pin> pinListDelete; ///< List of annotations to delete for client
	bool loadAnnotationsFlag; ///< Load annotations on start up
	std::string annotationsLoadPath; ///< File to load from
	UDPSocket *netSocket; ///< UDP Socket
	unsigned long sequenceNr; ///< UDP Packet sequence nr
};

VR_NAMESPACE_END
