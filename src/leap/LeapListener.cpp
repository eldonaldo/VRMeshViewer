#include "leap/LeapListener.hpp"

VR_NAMESPACE_BEGIN

using namespace Leap;
using namespace std;

LeapListener::LeapListener(bool useRift)
	: windowWidth(0.f), windowHeight(0.f), FBWidth(0.f), FBHeight(0.f), riftMounted(useRift) {
	fingerNames[0] = "Thumb"; fingerNames[1] = "Index"; fingerNames[2] = "Middle"; fingerNames[3] = "Ring"; fingerNames[4] = "Pinky";
	boneNames[0] = "Metacarpal"; boneNames[1] = "Proximal"; boneNames[2] = "Middle"; boneNames[3] = "Distal";
	stateNames[0] = "STATE_INVALID"; stateNames[1] = "STATE_START"; stateNames[2] = "STATE_UPDATE"; stateNames[3] = "STATE_END";
}

void LeapListener::onConnect(const Controller& controller) {
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

Vector LeapListener::leapToWorld (Vector &_v, InteractionBox &iBox, bool isRight, bool clamp) {
	if (riftMounted) {
		// Average of the left and right camera positions
		ovrPosef headPose = ovrHmd_GetTrackingState(hmd, 0).HeadPose.ThePose;
		OVR::Matrix4f trans = OVR::Matrix4f::Translation(headPose.Position);
		OVR::Matrix4f rot = OVR::Matrix4f(headPose.Orientation);

		// Rift to world transformation
		OVR::Matrix4f riftToWorld = trans * rot;

		// Encode the location (flip axis, rotation and translation) on the Rift where the Leap is mounted
		// x -> -x
		// y -> -z
		// z -> -y
		static const OVR::Matrix4f leapToRift(
			-1.f,  0.f,  0.f,  0.f,
			 0.f,  0.f, -1.f,  0.f,
			 0.f, -1.f,  0.f, -0.08f, // The VRMount is -8cm in front of the Leap
			 0.f,  0.f,  0.f,  1.f
		);

		// mm -> m
		static const OVR::Matrix4f mmTom(
			0.001f, 0.f,	0.f,	0.f,
			0.f,	0.001f, 0.f,	0.f,
			0.f,	0.f,	0.001f, 0.f, 
			0.f,	0.f,	0.f,	1.f
		);

		// Final transformation matrix that brings positions from Leap space into world-space (in meters)
		OVR::Matrix4f leapToWorld = riftToWorld * leapToRift * mmTom;

		// Transform point
		Vector4f v(_v.x, _v.y, _v.z, 1.f);
		Matrix4f leapToWorldEigen = Eigen::Map<Matrix4f>((float *)leapToWorld.M);
		Vector4f transformedV = leapToWorldEigen * v;

		static float scale = 35.f;
		return Leap::Vector(transformedV.x() * scale, transformedV.y() * scale, transformedV.z() * scale);
	} else {

		Vector normalizedPosition = iBox.normalizePoint(_v);
		Vector worldPosition;

		float scale = 4.f;
		Leap::Vector origin(-0.5f, -0.5f, -0.5f); // Middle of ibox
		float offset = isRight ? -0.25f : +0.25f;

		worldPosition = (normalizedPosition + origin);
		worldPosition.x += offset;
		worldPosition *= scale;

		//clamp after offsetting
		worldPosition.x = (clamp && worldPosition.x < 0) ? 0 : worldPosition.x;
		worldPosition.x = (clamp && worldPosition.x > 1) ? 1 : worldPosition.x;
		worldPosition.y = (clamp && worldPosition.y < 0) ? 0 : worldPosition.y;
		worldPosition.y = (clamp && worldPosition.y > 1) ? 1 : worldPosition.y;

		return worldPosition;
	}
}

void LeapListener::onFrame(const Controller &controller) {
	if (leftHand == nullptr || rightHand == nullptr)
		VRException("Leap hands not set! Call 'leapListener->setHands(hands[0], hands[1])'s");

	const Frame frame = controller.frame();
	PointableList pointables = frame.pointables();
	InteractionBox iBox = frame.interactionBox();

	/// For all available hands
	HandList handList = frame.hands();
	for (int i = 0; i < handList.count(); i++) {
		Hand hand = handList[i];
		Leap::Vector palmPosition = hand.palmPosition();
		Vector worldPalm = leapToWorld(palmPosition, iBox, hand.isRight());

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
