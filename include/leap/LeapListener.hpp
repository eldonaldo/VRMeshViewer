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
	LeapListener () = default;
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
	const std::string fingerNames[5] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
	const std::string boneNames[4] = { "Metacarpal", "Proximal", "Middle", "Distal" };
	const std::string stateNames[4] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };
};

VR_NAMESPACE_END
