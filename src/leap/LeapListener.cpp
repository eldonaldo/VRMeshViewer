#include "leap/LeapListener.hpp"

VR_NAMESPACE_BEGIN

using namespace Leap;
using namespace std;

LeapListener::LeapListener(bool useRift)
	: windowWidth(0.f), windowHeight(0.f), FBWidth(0.f), FBHeight(0.f), riftMounted(useRift), hmd(nullptr) {
	fingerNames[0] = "Thumb"; fingerNames[1] = "Index"; fingerNames[2] = "Middle"; fingerNames[3] = "Ring"; fingerNames[4] = "Pinky";
	boneNames[0] = "Metacarpal"; boneNames[1] = "Proximal"; boneNames[2] = "Middle"; boneNames[3] = "Distal";
	stateNames[0] = "STATE_INVALID"; stateNames[1] = "STATE_START"; stateNames[2] = "STATE_UPDATE"; stateNames[3] = "STATE_END";
}

Vector LeapListener::leapToWorld (Vector &v) {
	// No Rift transformation
	if (!riftMounted)
		return (v + Settings::getInstance().LEAP_TO_WORLD_ORIGIN) * Settings::getInstance().LEAP_TO_WORLD_SCALE_3D;

	if (hmd == nullptr)
		throw VRException("Could not transform Leap Vector to HMD - no Rift found!");

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
	Vector4f _v(v.x, v.y, v.z, 1.f);
	Matrix4f leapToWorldEigen = Eigen::Map<Matrix4f>((float *)leapToWorld.M);
	Vector3f &o = Settings::getInstance().CAMERA_OFFSET;
	Vector4f transformedV = (leapToWorldEigen * _v) + Vector4f(o.x(), o.y(), o.z(), 1.f);

	float scale = Settings::getInstance().LEAP_TO_WORLD_SCALE_HMD;
	return Leap::Vector(transformedV.x() * scale, transformedV.y() * scale, transformedV.z() * scale);
}

void LeapListener::onFrame(const Controller &controller) {
	if (leftHand == nullptr || rightHand == nullptr)
		VRException("Leap hands not set! Call 'leapListener->setHands(hands[0], hands[1])'s");

	const Frame frame = controller.frame();
	PointableList pointables = frame.pointables();
	InteractionBox iBox = frame.interactionBox();

	// For all available hands
	HandList hands = frame.hands();
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		const Hand hand = *hl;
		currentHand = leftHand;
		if (hand.isRight())
			currentHand = rightHand;

		// Palm position -> world position
		Leap::Vector palmPosition = hand.palmPosition();
		Vector worldPalm = leapToWorld(palmPosition);

		// Palm Rotation
		const Vector direction = hand.direction();
		const Vector plamNormal = hand.palmNormal();

//		std::cout << string(2, ' ') << "pitch: " << direction.pitch() * RAD_TO_DEG
//						<< " degrees, " << "roll: " << plamNormal.roll() * RAD_TO_DEG
//						<< " degrees, " << "yaw: " << direction.yaw() * RAD_TO_DEG
//						<< " degrees" << std::endl;

		Vector3f directionEigen(direction.x, direction.y, direction.z);
		Vector3f palmNormalEigen(plamNormal.x, plamNormal.y, plamNormal.z);
		Vector3f crossEigen = directionEigen.cross(palmNormalEigen);

		
//		currentHand->rotate(
//			plamNormal.roll(),
//			direction.pitch(),
//			direction.yaw()
//		);

		// Palm Translation
		currentHand->mesh.palm.translate(worldPalm.x, worldPalm.y, worldPalm.z);

		// For all fingers
		const FingerList fingers = hand.fingers();
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
			Leap::Vector tipPos = finger.tipPosition();
			Leap::Vector tipPosWorld = leapToWorld(tipPos);

			currentHand->mesh.finger[finger.type()].translate(tipPosWorld.x, tipPosWorld.y, tipPosWorld.z);
		}
	}
}

void LeapListener::onConnect(const Controller& controller) {
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
	Settings::getInstance().SHOW_HANDS = true;
}

void LeapListener::onDisconnect(const Controller &controller) {
	Settings::getInstance().SHOW_HANDS = false;
}

void LeapListener::onServiceConnect(const Controller &controller) {
	Settings::getInstance().SHOW_HANDS = true;
}

void LeapListener::onServiceDisconnect(const Controller &controller) {
	Settings::getInstance().SHOW_HANDS = false;
}

VR_NAMESPACE_END
