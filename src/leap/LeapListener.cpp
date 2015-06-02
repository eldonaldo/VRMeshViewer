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

	gestureZoom = GESTURE_STATES::STOP;
	gestureRotation = GESTURE_STATES::STOP;
}

Matrix4f LeapListener::getTransformationMatrix() {
	// Average of the left and right camera positions
	ovrPosef headPose = ovrHmd_GetTrackingState(hmd, 0).HeadPose.ThePose;

	// Add camera offset to the head position
	ovrVector3f cameraPose;
	cameraPose.x = headPose.Position.x + Settings::getInstance().CAMERA_OFFSET.x();
	cameraPose.y = headPose.Position.y + Settings::getInstance().CAMERA_OFFSET.y();
	cameraPose.z = headPose.Position.z + Settings::getInstance().CAMERA_OFFSET.z();

	OVR::Matrix4f trans = OVR::Matrix4f::Translation(cameraPose);
	OVR::Matrix4f rot = OVR::Matrix4f(headPose.Orientation);

	// Rift to world transformation
	OVR::Matrix4f riftToWorld = trans * rot;

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

	// mm -> m including zooming of factor Rift baseline / Leap baseline = 64mm / 40mm = 1.6mm / 1000 = 0.0016m
	float IPD = ovrHmd_GetFloat(hmd, OVR_KEY_IPD, 0.064f);
	float leapBaseline = 0.040f;
	float s = (IPD / leapBaseline) / 1000.f;

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

		// Get Rotation and translation matrix
		const Matrix4f worldTransform = getTransformationMatrix();
		const Matrix3f rotation = worldTransform.block<3, 3>(0, 0);
		const Vector3f translation = worldTransform.block<3, 1>(0, 3);

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

				// Transform palm
				const Vector3f palm = rotation * hand.palmPosition().toVector3<Vector3f>() + translation;
				const Vector3f palmDirection = (rotation * hand.direction().toVector3<Vector3f>()).normalized();
				const Vector3f palmNormal = (rotation * hand.palmNormal().toVector3<Vector3f>()).normalized();
				const Vector3f palmSide = palmDirection.cross(palmNormal).normalized();

				// Conjugation by rotation -> cancel out the length changes
				const Matrix3f palmRotation = rotation * (Matrix3f(hand.basis().toArray3x3())) * rotation.inverse();

				// Construct 4x4 rotation matrix
				Matrix4f rot(Matrix4f::Identity());
				rot.block<3, 3>(0, 0) = palmRotation;

				// Palm world properties : Angles
				currentHand->palm.pitch = hand.direction().pitch();
				currentHand->palm.yaw = hand.direction().yaw();
				currentHand->palm.roll = hand.palmNormal().roll();

				// Palm world properties : Positions
				currentHand->palm.position = Vector3f(palm.x(), palm.y(), palm.z());
				currentHand->palm.direction = palmDirection;
				currentHand->palm.normal = palmNormal;
				currentHand->palm.side = palmSide;

				// Transform
				currentHand->mesh.palm.translate(palm.x(), palm.y(), palm.z());
				currentHand->mesh.palm.setRotationMatrix(rot);

				// For all fingers
				const FingerList fingers = hand.fingers();
				for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
					const Finger finger = *fl;

					// Finger tip world position
					Vector3f tip = rotation * finger.tipPosition().toVector3<Vector3f>() + translation;
					Vector3f direction = rotation * finger.direction().toVector3<Vector3f>();
					currentHand->finger[finger.type()].position = Vector3f(tip.x(), tip.y(), tip.z());
					currentHand->finger[finger.type()].extended = finger.isExtended() && hand.grabStrength() <= 0.5f;
					currentHand->finger[finger.type()].direction = direction;

					// Transform
					currentHand->mesh.finger[finger.type()].translate(tip.x(), tip.y(), tip.z());
					currentHand->mesh.finger[finger.type()].setRotationMatrix(rot);
				}
			}

			// Reset tracking states if no hands found
			if (hands.count() == 0) {
				for (auto &h : skeletonHands)
					h->confidence = h->pinchStrength = h->grabStrength = 0.f;

				gestureZoom = GESTURE_STATES::STOP;
			}

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

				gestureZoom = GESTURE_STATES::STOP;
			}
		}

		// Process built in Leap gestures
		const GestureList gestures = frame.gestures();
		for (int g = 0; g < gestures.count(); g++) {
			Gesture gesture = gestures[g];
			switch (gesture.type()) {
				case Gesture::TYPE_SWIPE: {
					SwipeGesture swipe = gesture;
					Gesture::State leapState = swipe.state();

					if (swipe.hands().count() == 1)
						gestureHandler->swipe(leapToInternState(leapState), skeletonHands, swipe);

					break;
				}

				case Gesture::TYPE_SCREEN_TAP: {
					ScreenTapGesture tap = gesture;
					Gesture::State leapState = tap.state();
					gestureHandler->screenTap(leapToInternState(leapState), skeletonHands, tap);
					break;
				}

				case Gesture::TYPE_CIRCLE:
				case Gesture::TYPE_INVALID:
				case Gesture::TYPE_KEY_TAP:
				default:
					break;
			}
		}
	}

	// Process own built gesture state machines
	gesturesStateMachines();
}

void LeapListener::gesturesStateMachines() {
	// for both hands
	for (int i = 0; i < 2; i++) {
		auto &hand = skeletonHands[i];

		/**
		 * Pinch state machine
		 */
		float pinchThreshold = Settings::getInstance().GESTURES_PINCH_THRESHOLD;

		if (hand->pinchStrength >= pinchThreshold && gestures[i][GESTURES::PINCH] == GESTURE_STATES::STOP) {
			// Start
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::START;
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
		}
		else if (hand->pinchStrength >= pinchThreshold && (gestures[i][GESTURES::PINCH] == GESTURE_STATES::START || gestures[i][GESTURES::PINCH] == GESTURE_STATES::UPDATE)) {
			// Update
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::UPDATE;
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
		}
		else if (hand->pinchStrength < pinchThreshold && gestures[i][GESTURES::PINCH] == GESTURE_STATES::UPDATE) {
			// Stop
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::STOP;
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
		}

		/**
		* Grab state machine
		*/
		float grabThreshold = Settings::getInstance().GESTURES_PINCH_THRESHOLD;
		if (hand->grabStrength >= grabThreshold && gestures[i][GESTURES::GRAB] == GESTURE_STATES::STOP) {
			// Start
			gestures[i][GESTURES::GRAB] = GESTURE_STATES::START;
			gestureHandler->grab(gestures[i][GESTURES::GRAB], (HANDS)i, skeletonHands);
		}
		else if (hand->grabStrength >= grabThreshold && (gestures[i][GESTURES::GRAB] == GESTURE_STATES::START || gestures[i][GESTURES::GRAB] == GESTURE_STATES::UPDATE)) {
			// Update
			gestures[i][GESTURES::GRAB] = GESTURE_STATES::UPDATE;
			gestureHandler->grab(gestures[i][GESTURES::GRAB], (HANDS)i, skeletonHands);
		}
		else if (hand->grabStrength < grabThreshold && gestures[i][GESTURES::GRAB] == GESTURE_STATES::UPDATE) {
			// Stop
			gestures[i][GESTURES::GRAB] = GESTURE_STATES::STOP;
			gestureHandler->grab(gestures[i][GESTURES::GRAB], (HANDS)i, skeletonHands);
		}
	}
	
	/**
	* Zoom  and rotation gesture state machine
	*
	* The gestureHandler->rotate internally calls the zoom method.
	*/
	unsigned int extendedCount = 0;
	for (int i = 0; i < 5; i++) {
		if (leftHand->finger[i].extended && rightHand->finger[i].extended)
			extendedCount++;
	}

	if (extendedCount == 5 && gestureRotation == GESTURE_STATES::STOP) {
		gestureHandler->rotate(GESTURE_STATES::START, skeletonHands);
		gestureRotation = GESTURE_STATES::START;
	}
	else if (extendedCount == 5 && (gestureRotation == GESTURE_STATES::START || gestureRotation == GESTURE_STATES::UPDATE)) {
		gestureHandler->rotate(GESTURE_STATES::UPDATE, skeletonHands);
		gestureRotation = GESTURE_STATES::UPDATE;
	}
	else if (extendedCount < 5 && gestureRotation == GESTURE_STATES::UPDATE) {
		gestureHandler->rotate(GESTURE_STATES::STOP, skeletonHands);
		gestureRotation = GESTURE_STATES::STOP;
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

GESTURE_STATES LeapListener::leapToInternState (Leap::Gesture::State &s) {
	GESTURE_STATES state = GESTURE_STATES::INVALID;
	switch (s) {
		case Leap::Gesture::State::STATE_INVALID:
			state = GESTURE_STATES::INVALID;
			break;

		case Leap::Gesture::State::STATE_START:
			state = GESTURE_STATES::START;
			break;

		case Leap::Gesture::State::STATE_UPDATE:
			state = GESTURE_STATES::UPDATE;
			break;

		case Leap::Gesture::State::STATE_STOP:
			state = GESTURE_STATES::STOP;
			break;
	}

	return state;
}

VR_NAMESPACE_END
