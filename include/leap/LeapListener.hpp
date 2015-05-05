#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"
#include "Leap.h"

VR_NAMESPACE_BEGIN

/**
 * @brief Leap listener
 *
 * Implements the leap API
 */
class LeapListener : public Leap::Listener {
public:
	LeapListener ();
	virtual ~LeapListener () = default;

	/**
	 * Leap listener implementation
	 */
	virtual void onInit (const Leap::Controller &controller);
	virtual void onConnect (const Leap::Controller &controller);
	virtual void onDisconnect (const Leap::Controller &controller);
	virtual void onExit (const Leap::Controller &controller);
	virtual void onFrame (const Leap::Controller &controller);
	virtual void onFocusGained (const Leap::Controller &controller);
	virtual void onFocusLost (const Leap::Controller &controller);
	virtual void onDeviceChange (const Leap::Controller &controller);
	virtual void onServiceConnect (const Leap::Controller &controller);
	virtual void onServiceDisconnect (const Leap::Controller &controller);

	/**
	 * @brief Updates the window and framebuffer sizes
	 */
	void setSize (float windowWidth, float windowHeight, float FBWidth, float FBHeight) {
		this->windowHeight = windowWidth; this->windowHeight = windowHeight;
		this->FBWidth = FBWidth; this->FBHeight = FBHeight;
	}

	/**
	 * @brief Set the leap hands
	 */
	void setHands (std::shared_ptr<Mesh> &l, std::shared_ptr<Mesh> &r) {
		leftHand = l; rightHand = r;
	}

protected:

	/**
	 * @brief Transforms a Leap vector into a Leap world vector
	 *
	 * @param v Leap vector
	 * @param ibox Leap interaction box
	 * @param isRight Right or left hand
	 * @return Leap world vector
	 */
	Leap::Vector leapToWorld (Leap::Vector v, Leap::InteractionBox &ibox, bool isRight);

	/**
	 * @brief Leap to Eigen conversion
	 */
	Vector3f leapToEigen (Leap::Vector v) {
		return Vector3f(v.x, v.y, v.z);
	}

protected:
	float scale; ///< Coordinate system scale factor
	Leap::Vector worldOrigin; ///< World coordinate system origin
	Leap::Vector leftOrigin, rightOrigin; ///< Leap/world coordinate system origin
	std::string fingerNames[5]; ///< Finger names
	std::string boneNames[4]; ///< Bone names
	std::string stateNames[4]; ///< Leap states
	float windowWidth, windowHeight; ///y GLFW window size
	float FBWidth, FBHeight; ///< Framebuffer size
	std::shared_ptr<Mesh> leftHand, rightHand; ///< Leap hands
};

VR_NAMESPACE_END
