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
	LeapListener();
	virtual ~LeapListener () = default;
	virtual void onInit (const Leap::Controller&);
	virtual void onConnect (const Leap::Controller&);
	virtual void onDisconnect (const Leap::Controller&);
	virtual void onExit (const Leap::Controller&);
	virtual void onFrame (const Leap::Controller&);
	virtual void onFocusGained (const Leap::Controller&);
	virtual void onFocusLost (const Leap::Controller&);
	virtual void onDeviceChange (const Leap::Controller&);
	virtual void onServiceConnect (const Leap::Controller&);
	virtual void onServiceDisconnect (const Leap::Controller&);

protected:
	std::string fingerNames[5];
	std::string boneNames[4];
	std::string stateNames[4];
};

VR_NAMESPACE_END
