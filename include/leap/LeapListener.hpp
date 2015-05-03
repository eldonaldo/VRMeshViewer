#pragma once

#include "common.hpp"
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

	void setSize (float windowWidth, float windowHeight, float FBWidth, float FBHeight) {
		this->windowHeight = windowWidth;
		this->windowHeight = windowHeight;
		this->FBWidth = FBWidth;
		this->FBHeight = FBHeight;
	}

protected:

	Leap::Vector leapToWorld (Leap::Vector v, Leap::InteractionBox &ibox);

protected:
	Leap::Vector worldOrigin;
	float scale;
	std::string fingerNames[5];
	std::string boneNames[4];
	std::string stateNames[4];
	float windowWidth, windowHeight;
	float FBWidth, FBHeight;
};

VR_NAMESPACE_END
