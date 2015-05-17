#include "leap/LeapListener.hpp"

VR_NAMESPACE_BEGIN

using namespace Leap;
using namespace std;

LeapListener::LeapListener(bool useRift)
	: windowWidth(0.f), windowHeight(0.f), FBWidth(0.f), FBHeight(0.f), riftMounted(useRift), hmd(nullptr) {

	// A List of all gestures an their initial state, 0 = right hand, 1 = left hand
	for (int i = 0; i < 2; i++) {
		gestures[i][GESTURES::PINCH] = GESTURE_STATES::STOP;
	}
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

	// Only on valid frames
	const Frame frame = controller.frame();
	if (frame.isValid()) {

		// Only handle 1 or 2 hands, but not more
		HandList hands = frame.hands();
		if (hands.count() <= 2) {

			// Processing
			for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); hl++) {
				const Hand hand = *hl;
				currentHand = leftHand;
				if (hand.isRight())
					currentHand = rightHand;

				// Collect tracking state
				currentHand->id = hand.id();
				currentHand->confidence = hand.confidence();
				currentHand->pinchStrength = hand.pinchStrength();
				currentHand->grabStrength = hand.grabStrength();

				// Get Rotation and translation matrix
				const Matrix4f worldTransform = getTransformationMatrix();
				const Matrix3f rotation = worldTransform.block<3, 3>(0, 0);
				const Vector3f translation = worldTransform.block<3, 1>(0, 3);

				// Transform palm
				const Vector3f palm = rotation * hand.palmPosition().toVector3<Vector3f>() + translation;
				const Matrix3f palmRotation = rotation * (Matrix3f(hand.basis().toArray3x3())) * rotation.inverse();
	//			const Vector3f palmDir = (rotation * hand.direction().toVector3<Vector3f>()).normalized();
	//			const Vector3f palmNormal = (rotation * hand.palmNormal().toVector3<Vector3f>()).normalized();
	//			const Vector3f palmSide = palmDir.cross(palmNormal).normalized();
	//			const Matrix3f palmBasis = rotation * (Matrix3f(hand.basis().toArray3x3()));

				// Construct 4x4 rotation matrix
				Matrix4f rot(Matrix4f::Identity());
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
					currentHand->fingerPosition[finger.type()] = Vector3f(tip.x(), tip.y(), tip.z());
					currentHand->mesh.finger[finger.type()].translate(tip.x(), tip.y(), tip.z());
					currentHand->mesh.finger[finger.type()].setRotationMatrix(rot);
				}
			}

			// Reset tracking states if no hands found
			if (hands.count() == 0)
				for (auto &h : skeletonHands)
					h->confidence = h->pinchStrength = h->grabStrength = 0.f;

			// Reset tracking state of the other hand
			if (hands.count() == 1) {
				// We want to reset the state of the other hand ...
				int handIndex = HANDS::LEFT;
				currentHand = leftHand;
				if (hands[0].isLeft()) {
					currentHand = rightHand;
					handIndex = HANDS::RIGHT;
				}

				for (auto &g : gestures[handIndex])
					g.second = GESTURE_STATES::STOP;
			}
		}
	}

	// Do something useful with it ...
	recognizeGestures();
}

void LeapListener::recognizeGestures() {
	// for both hands
	for (int i = 0; i < 2; i++) {
		auto &hand = skeletonHands[i];

		/**
		 * Pinch -> ranges between [0, 1]
		 */
		float pinchThreshold = Settings::getInstance().GESTURES_PINCH_THRESHOLD;

		// Start
		if (hand->pinchStrength > pinchThreshold && gestures[i][GESTURES::PINCH] == GESTURE_STATES::STOP) {
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::START;
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
		}
		else if (hand->pinchStrength > pinchThreshold && (gestures[i][GESTURES::PINCH] == GESTURE_STATES::START || gestures[i][GESTURES::PINCH] == GESTURE_STATES::UPDATE)) {
			// Update
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::UPDATE;
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
		}
		else if (hand->pinchStrength < pinchThreshold && gestures[i][GESTURES::PINCH] == GESTURE_STATES::UPDATE) {
			// Stop
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::STOP;
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
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

void LeapListener::onInit(const Leap::Controller &controller) {

}

void LeapListener::onExit(const Leap::Controller &controller) {

}

void LeapListener::onDeviceChange (const Leap::Controller &controller) {

}

void LeapListener::setSize (float windowWidth, float windowHeight, float FBWidth, float FBHeight) {
	this->windowHeight = windowWidth; this->windowHeight = windowHeight;
	this->FBWidth = FBWidth; this->FBHeight = FBHeight;
}

void LeapListener::setHands (std::shared_ptr<SkeletonHand> &l, std::shared_ptr<SkeletonHand> &r) {
	skeletonHands[0] = rightHand = r;
	skeletonHands[1] = leftHand = l;
}

void LeapListener::setHmd(ovrHmd h) {
	hmd = h;
}

void LeapListener::setGestureHandler (std::shared_ptr<GestureHandler> &s) {
	gestureHandler = s;
}

VR_NAMESPACE_END
