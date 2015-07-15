#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"
#include "Leap.h"
#include "leap/SkeletonHand.hpp"
#include "leap/GestureHandler.hpp"
#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief Leap listener
 *
 * Implements the leap API
 */
class LeapListener : public Leap::Listener {
public:
	LeapListener();
	virtual ~LeapListener () = default;

	/**
	 * Leap listener implementation
	 */
	virtual void onInit(const Leap::Controller &controller);
	virtual void onConnect (const Leap::Controller &controller);
	virtual void onDisconnect (const Leap::Controller &controller);
	virtual void onExit(const Leap::Controller &controller);
	virtual void onFrame (const Leap::Controller &controller);
	virtual Leap::Frame pollFrame (const Leap::Controller &controller);
	virtual void onServiceConnect (const Leap::Controller &controller);
	virtual void onServiceDisconnect (const Leap::Controller &controller);
	virtual void onDeviceChange (const Leap::Controller &controller);

	/**
	* @brief Recognizes gestures
	*/
	void gesturesStateMachines();

	/**
	 * @brief Set the leap hands
	 */
	void setHands (std::shared_ptr<SkeletonHand> &l, std::shared_ptr<SkeletonHand> &r);

	/**
	* @brief Sets the pointer to the Hmd
	*/
	void setHmd (ovrHmd h);

	/**
	* @brief Sets mesh pointer
	*/
	void setMesh(std::shared_ptr<Mesh> &m);

	/**
	 * @brief Sets the gesture handler
	 */
	void setGestureHandler (std::shared_ptr<GestureHandler> &s);

protected:

	/**
	* @brief Returns the transformation matrix from Leap to World coordinates
	*/
	Matrix4f getTransformationMatrix();

	/**
	 * @brief Leap state to intern state
	 */
	GESTURE_STATES leapToInternState (Leap::Gesture::State &s);

	void stopRotationGesture(int hand = -1);
	void stopZoomGesture();
	void stopPinchGensture(int hand = -1);

protected:

	float windowWidth, windowHeight; ///y GLFW window size
	float FBWidth, FBHeight; ///< Framebuffer size
	std::shared_ptr<SkeletonHand> leftHand, rightHand, currentHand; ///< Leap hands
	std::shared_ptr<SkeletonHand> skeletonHands[2]; ///< Pointer to the hands above
	std::map<GESTURES, GESTURE_STATES> gestures[2]; ///< Gesture states (for both hands)
	GESTURE_STATES gestureZoom; ///< Zoom gesture states, not applicapable to one hand
	ovrHmd hmd; ///< The Rift
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<GestureHandler> gestureHandler; ///< Gesture handler
};

VR_NAMESPACE_END
