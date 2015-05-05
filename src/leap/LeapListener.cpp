#include "leap/LeapListener.hpp"

VR_NAMESPACE_BEGIN

using namespace Leap;
using namespace std;

LeapListener::LeapListener ()
	: windowWidth(0.f), windowHeight(0.f), FBWidth(0.f), FBHeight(0.f) {
	fingerNames[0] = "Thumb"; fingerNames[1] = "Index"; fingerNames[2] = "Middle"; fingerNames[3] = "Ring"; fingerNames[4] = "Pinky";
	boneNames[0] = "Metacarpal"; boneNames[1] = "Proximal"; boneNames[2] = "Middle"; boneNames[3] = "Distal";
	stateNames[0] = "STATE_INVALID"; stateNames[1] = "STATE_START"; stateNames[2] = "STATE_UPDATE"; stateNames[3] = "STATE_END";

	// Coordinate system change settings
	worldOrigin = Leap::Vector( 0.f, 0.f, 0.f);
	rightOrigin = Leap::Vector(+2.f, 0.f, 0.f);
	leftOrigin  = Leap::Vector(-2.f, 0.f, 0.f);
	scale = 5.f;
}

void LeapListener::onConnect(const Controller& controller) {
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

Vector LeapListener::leapToWorld (Vector v, InteractionBox &iBox, bool isRight = true) {
	Vector normalizedPalmPosition = iBox.normalizePoint(v);
	Vector worldPalmPosition;

	if (isRight)
		worldPalmPosition = (normalizedPalmPosition + worldOrigin) * scale + rightOrigin;
	else
		worldPalmPosition = (normalizedPalmPosition + worldOrigin) * scale + leftOrigin;

	return worldPalmPosition;
}

void LeapListener::onFrame(const Controller &controller) {
	if (leftHand == nullptr || rightHand == nullptr)
		VRException("Leap hands not set! Call 'leapListener->setHands(hands[0], hands[1])'s");

	const Frame frame = controller.frame();
	PointableList pointables = frame.pointables();
	InteractionBox iBox = frame.interactionBox();

	HandList handList = frame.hands();
	for (int i = 0; i < handList.count(); i++) {
		Hand hand = handList[i];
		Vector worldPalm = leapToWorld(hand.palmPosition(), iBox, hand.isRight());

		cout << worldPalm << endl;

		if (hand.isRight())
			rightHand->translate(worldPalm.x, worldPalm.y, worldPalm.z);
		else
			leftHand->translate(worldPalm.x, worldPalm.y, worldPalm.z);
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
