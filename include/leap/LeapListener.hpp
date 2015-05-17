#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"
#include "Leap.h"
#include "leap/SkeletonHand.hpp"
#include "leap/GestureHandler.hpp"

VR_NAMESPACE_BEGIN



/**
 * @brief Leap listener
 *
 * Implements the leap API
 */
class LeapListener : public Leap::Listener {
public:
	LeapListener(bool useRift);
	virtual ~LeapListener () = default;

	/**
	 * Leap listener implementation
	 */
	virtual void onInit(const Leap::Controller &controller);
	virtual void onConnect (const Leap::Controller &controller);
	virtual void onDisconnect (const Leap::Controller &controller);
	virtual void onExit(const Leap::Controller &controller);
	virtual void onFrame (const Leap::Controller &controller);
	virtual void onServiceConnect (const Leap::Controller &controller);
	virtual void onServiceDisconnect (const Leap::Controller &controller);
	virtual void onDeviceChange (const Leap::Controller &controller);

	/**
	* @brief Recognizes gestures
	*/
	void recognizeGestures();

	/**
	 * @brief Updates the window and framebuffer sizes
	 */
	void setSize (float windowWidth, float windowHeight, float FBWidth, float FBHeight);

	/**
	 * @brief Set the leap hands
	 */
	void setHands (std::shared_ptr<SkeletonHand> &l, std::shared_ptr<SkeletonHand> &r);

	/**
	* @brief Sets the pointer to the Hmd
	*/
	void setHmd(ovrHmd h);

	/**
	 * @brief Sets the gesture handler
	 */
	void setGestureHandler (std::shared_ptr<GestureHandler> &s);

protected:

	/**
	* @brief Returns the transformation matrix from Leap to World coordinates
	*/
	Matrix4f getTransformationMatrix();

protected:

	bool riftMounted; ///< Leap on HMD?
	float windowWidth, windowHeight; ///y GLFW window size
	float FBWidth, FBHeight; ///< Framebuffer size
	std::shared_ptr<SkeletonHand> leftHand, rightHand, currentHand; ///< Leap hands
	std::shared_ptr<SkeletonHand> skeletonHands[2]; ///< Pointer to the hands above
	std::map<GESTURES, GESTURE_STATES> gestures[2]; ///< Gesture states
	ovrHmd hmd; ///< The Rift
	std::shared_ptr<GestureHandler> gestureHandler; ///< Gesture handler
};

VR_NAMESPACE_END
