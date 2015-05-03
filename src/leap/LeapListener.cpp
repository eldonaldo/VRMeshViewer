#include "leap/LeapListener.hpp"

VR_NAMESPACE_BEGIN

using namespace Leap;
using namespace std;

LeapListener::LeapListener ()
	: worldOrigin(0.5, 0.f, 0.5f), scale(100.f), windowWidth(0.f), windowHeight(0.f), FBWidth(0.f), FBHeight(0.f) {
	fingerNames[0] = "Thumb";
	fingerNames[1] = "Index";
	fingerNames[2] = "Middle";
	fingerNames[3] = "Ring";
	fingerNames[4] = "Pinky";

	boneNames[0] = "Metacarpal";
	boneNames[1] = "Proximal";
	boneNames[2] = "Middle";
	boneNames[3] = "Distal";

	stateNames[0] = "STATE_INVALID";
	stateNames[1] = "STATE_START";
	stateNames[2] = "STATE_UPDATE";
	stateNames[3] = "STATE_END";
}

void LeapListener::onConnect(const Controller& controller) {
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

Vector LeapListener::leapToWorld (Vector v, InteractionBox &iBox) {
	Vector normalizedPalmPosition = iBox.normalizePoint(v);
	Vector worldPalmPosition = (normalizedPalmPosition + worldOrigin) * scale;
	return worldPalmPosition;
}

void LeapListener::onFrame(const Controller &controller) {
	const Frame frame = controller.frame();
	PointableList pointables = frame.pointables();
	InteractionBox iBox = frame.interactionBox();

	HandList handList = frame.hands();
	for (int i = 0; i < handList.count(); i++) {
		Hand hand = handList[i];
		Vector worldPalm = leapToWorld(hand.palmPosition(), iBox);
	}
}

void LeapListener::onInit(const Controller &controller) {

}

void LeapListener::onDisconnect(const Controller &controller) {

}

void LeapListener::onExit(const Controller &controller) {

}

void LeapListener::onFocusGained(const Controller &controller) {

}

void LeapListener::onFocusLost(const Controller &controller) {

}

void LeapListener::onDeviceChange(const Controller &controller) {

}

void LeapListener::onServiceConnect(const Controller &controller) {

}

void LeapListener::onServiceDisconnect(const Controller &controller) {

}

VR_NAMESPACE_END
