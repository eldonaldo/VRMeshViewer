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

Matrix4f LeapListener::getTransformationMatrix() {
	// Average of the left and right camera positions
	ovrPosef headPose = ovrHmd_GetTrackingState(hmd, 0).HeadPose.ThePose;

	// Add camera offset the head position
	ovrVector3f C;
	C.x = headPose.Position.x + Settings::getInstance().CAMERA_OFFSET.x();
	C.y = headPose.Position.y + Settings::getInstance().CAMERA_OFFSET.y();
	C.z = headPose.Position.z + Settings::getInstance().CAMERA_OFFSET.z();

	OVR::Matrix4f trans = OVR::Matrix4f::Translation(C);
	OVR::Matrix4f rot = OVR::Matrix4f(headPose.Orientation);

	// Rift to world transformation
	OVR::Matrix4f riftToWorld = trans *rot;

	// Encode the location (flip axis, rotation and translation) on the Rift where the Leap is mounted
	// x -> -x
	// y -> -z
	// z -> -y
	static const OVR::Matrix4f leapToRift(
		-1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, -1.f, 0.f, Settings::getInstance().LEAP_CAMERA_SHIFT_Z, // The VRMount is -8cm in front of the Leap
		0.f, 0.f, 0.f, 1.f
	);

	// mm -> m including zooming of factor Rift basline / Leap baseline = 0.064f / 40.0f
	float s = 0.064f / 40.0f;
	static const OVR::Matrix4f mmTom(
		s, 0.f, 0.f, 0.f,
		0.f, s, 0.f, 0.f,
		0.f, 0.f, s, 0.f,
		0.f, 0.f, 0.f, 1.f
	);

	// Final transformation matrix that brings positions from Leap space into world-space (in meters)
	OVR::Matrix4f leapToWorld = riftToWorld * leapToRift * mmTom;
	Matrix4f T = Eigen::Map<Matrix4f>((float *)leapToWorld.Transposed().M);

	return T;
}

void LeapListener::onFrame(const Controller &controller) {
	if (leftHand == nullptr || rightHand == nullptr)
		VRException("Leap hands not set! Call 'leapListener->setHands(hands[0], hands[1])'s");

	const Frame frame = controller.frame();
	if (frame.isValid()) {

		// For all available hands
		HandList hands = frame.hands();
		for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); hl++) {
			const Hand hand = *hl;
			currentHand = leftHand;
			if (hand.isRight())
				currentHand = rightHand;

			currentHand->id = hand.id();
			currentHand->confidence = hand.confidence();
			currentHand->pinchStrength = hand.pinchStrength();
			currentHand->grabStrength = hand.grabStrength();

			static bool t = false;
			if (!t && currentHand->pinchStrength > 0.9) {
				cout << "pinch start" << endl;
				t = true;
			}

			if (t && currentHand->pinchStrength < 0.9) {
				cout << "pinch end" << endl;
				t = false;
			}

			// Get Rotation and translation matrix
			const Matrix4f worldTransform = getTransformationMatrix();
			const Matrix3f rotation = worldTransform.block<3, 3>(0, 0);
			const Vector3f translation = worldTransform.block<3, 1>(0, 3);

			// Transform palm
			const Vector3f palm = rotation * hand.palmPosition().toVector3<Vector3f>() + translation;
			const Vector3f palmDir = (rotation * hand.direction().toVector3<Vector3f>()).normalized();
			const Vector3f palmNormal = (rotation * hand.palmNormal().toVector3<Vector3f>()).normalized();
			const Vector3f palmSide = palmDir.cross(palmNormal).normalized();
			const Matrix3f palmRotation = rotation * (Matrix3f(hand.basis().toArray3x3())) * rotation.inverse();
			const Matrix3f palmBasis = rotation * (Matrix3f(hand.basis().toArray3x3()));

			// Construct 4x4 rotation matrix
			Matrix4f rot = Matrix4f::Identity();
			rot.block<3, 3>(0, 0) = palmRotation;

			// Palm world position
			currentHand->palmPosition = Vector3f(palm.x(), palm.y(), palm.z());
			currentHand->mesh.palm.translate(currentHand->palmPosition.x(), currentHand->palmPosition.y(), currentHand->palmPosition.z());
			currentHand->mesh.palm.setRotationMatrix(rot);

			// For all fingers
			const FingerList fingers = hand.fingers();
			for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
				const Finger finger = *fl;

				// Finger tip world position
				Vector3f tip = rotation * finger.tipPosition().toVector3<Vector3f>() + translation;
				currentHand->mesh.finger[finger.type()].translate(tip.x(), tip.y(), tip.z());
				currentHand->mesh.finger[finger.type()].setRotationMatrix(rot);
			}
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
