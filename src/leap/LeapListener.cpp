#include "leap/LeapListener.hpp"

VR_NAMESPACE_BEGIN

using namespace Leap;
using namespace std;

LeapListener::LeapListener(bool useRift)
	: windowWidth(0.f), windowHeight(0.f), FBWidth(0.f), FBHeight(0.f), riftMounted(useRift), hmd(nullptr) {

	// A List of all gestures an their initial state, 0 = right hand, 1 = left hand
	for (int i = 0; i < 2; i++) {
		gestures[i][GESTURES::PINCH] = GESTURE_STATES::STOP;
		gestures[i][GESTURES::ROTATION] = GESTURE_STATES::STOP;
	}

	gestureZoom = GESTURE_STATES::STOP;
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

	const Frame frame = controller.frame();

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
			currentHand->visible = true;

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
			for (int i = 0; i < 5; i++) {
				const Finger &finger = hand.fingers()[i];

				// Finger tip world position
				Vector3f tip = rotation * finger.tipPosition().toVector3<Vector3f>() + translation;
				Vector3f direction = rotation * finger.direction().toVector3<Vector3f>();
				currentHand->finger[finger.type()].position = Vector3f(tip.x(), tip.y(), tip.z());
				currentHand->finger[finger.type()].extended = finger.isExtended();
				currentHand->finger[finger.type()].direction = direction;

				// Transform
				currentHand->mesh.finger[finger.type()].translate(tip.x(), tip.y(), tip.z());
				currentHand->mesh.finger[finger.type()].setRotationMatrix(rot);

				// Bones
				for (int k = 0; k < currentHand->mesh.nrOfJoints; k++) {
					// Joints
					Leap::Bone bone = finger.bone(static_cast<Leap::Bone::Type>(k));
					Vector3f jointPosition = rotation * bone.nextJoint().toVector3<Vector3f>() + translation;
					currentHand->finger[finger.type()].jointPositions[k] = Vector3f(jointPosition.x(), jointPosition.y(), jointPosition.z());
					currentHand->mesh.joints[i][k].translate(jointPosition.x(), jointPosition.y(), jointPosition.z());
					currentHand->mesh.joints[i][k].setRotationMatrix(rot);

					// Closing joint for metacarpal and proxicarpal
					if (finger.type() == Finger::Type::TYPE_PINKY && k == 0) {
						Vector3f handJointPos = rotation * bone.prevJoint().toVector3<Vector3f>() + translation;
						currentHand->handJointPosition = handJointPos;
						currentHand->mesh.handJoint.translate(handJointPos.x(), handJointPos.y(), handJointPos.z());
						currentHand->mesh.handJoint.setRotationMatrix(rot);
					}
				}

			}
		}

		// Reset tracking states if no hands found
		if (hands.count() == 0) {
			for (auto &h : skeletonHands) {
				h->confidence = h->pinchStrength = h->grabStrength = 0.f;
				h->visible = false;
				for (auto &f : h->finger)
					f.extended = false;
			}

			for (int i = 0; i < 2; i++)
				for (auto &g : gestures[i])
					g.second = GESTURE_STATES::STOP;

			gestureZoom = GESTURE_STATES::STOP;
		}

		// Reset tracking state of the other hand
		if (hands.count() == 1) {
			// We want to reset the state of the other hand ...
			int handIndex = HANDS::LEFT;
			currentHand = leftHand;
			if (hands[0].isLeft()) {
				currentHand = rightHand;
				currentHand->visible = false;
				handIndex = HANDS::RIGHT;
			}

			for (auto &g : gestures[handIndex])
				g.second = GESTURE_STATES::STOP;

			gestureZoom = GESTURE_STATES::STOP;

			for (auto &f : currentHand->finger)
				f.extended = false;
		}
	}

	// Process own built gesture state machines
	gesturesStateMachines();
}

void LeapListener::gesturesStateMachines() {
	/**
	* Zoom gesture state machine
	*/
	unsigned int extendedCount = 0;
	for (int i = 0; i < 5; i++) {
		if ((leftHand->visible && (leftHand->finger[i].extended || (!leftHand->finger[i].extended && leftHand->grabStrength <= 0.6f))) &&
			(rightHand->visible && (rightHand->finger[i].extended || (!rightHand->finger[i].extended && rightHand->grabStrength <= 0.6f))))
			extendedCount++;
	}

	Vector3f sphereCenter = mesh->getBoundingBox().getCenter();
	float sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * 0.5f;
	Vector3f midLeft = (leftHand->palm.position + leftHand->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f midRight = (rightHand->palm.position + rightHand->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f p1 = midLeft - sphereCenter;
	Vector3f p2 = midRight - sphereCenter;
	bool p1Inside = powf((p1.x()), 2.f) + powf((p1.y()), 2.f) + powf((p1.z()), 2.f) <= powf((sphereRadius), 2.f);
	bool p2Inside = powf((p2.x()), 2.f) + powf((p2.y()), 2.f) + powf((p2.z()), 2.f) <= powf((sphereRadius), 2.f);
	bool insideSphere = p1Inside && p2Inside;

	if (insideSphere && extendedCount == 5 && gestureZoom == GESTURE_STATES::STOP) {
		gestureHandler->scale(GESTURE_STATES::START, skeletonHands);
		gestureZoom = GESTURE_STATES::START;

		gestures[HANDS::LEFT][GESTURES::ROTATION] = GESTURE_STATES::STOP;
		gestures[HANDS::RIGHT][GESTURES::ROTATION] = GESTURE_STATES::STOP;
	}
	else if (extendedCount == 5 && (gestureZoom == GESTURE_STATES::START || gestureZoom == GESTURE_STATES::UPDATE)) {
		gestureHandler->scale(GESTURE_STATES::UPDATE, skeletonHands);
		gestureZoom = GESTURE_STATES::UPDATE;

		gestures[HANDS::LEFT][GESTURES::ROTATION] = GESTURE_STATES::STOP;
		gestures[HANDS::RIGHT][GESTURES::ROTATION] = GESTURE_STATES::STOP;
	}
	else if (extendedCount < 5) {
		gestureHandler->scale(GESTURE_STATES::STOP, skeletonHands);
		gestureZoom = GESTURE_STATES::STOP;
	}

	// for both hands
	for (int i = 0; i < 2; i++) {
		auto &hand = skeletonHands[i];

		/**
		 * Pinch state machine
		 */
		float pinchThreshold = Settings::getInstance().GESTURES_PINCH_THRESHOLD;

		if (hand->visible && hand->pinchStrength >= pinchThreshold && gestures[i][GESTURES::PINCH] == GESTURE_STATES::STOP) {
			// Start
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::START;

			gestures[i][GESTURES::ROTATION] = GESTURE_STATES::STOP;
			gestureZoom = GESTURE_STATES::STOP;
		}
		else if (hand->visible && hand->pinchStrength >= pinchThreshold && (gestures[i][GESTURES::PINCH] == GESTURE_STATES::START || gestures[i][GESTURES::PINCH] == GESTURE_STATES::UPDATE)) {
			// Update
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::UPDATE;

			gestures[i][GESTURES::ROTATION] = GESTURE_STATES::STOP;
			gestureZoom = GESTURE_STATES::STOP;
		}
		else if (hand->visible && hand->pinchStrength < pinchThreshold) {
			// Stop
			gestureHandler->pinch(gestures[i][GESTURES::PINCH], (HANDS) i, skeletonHands);
			gestures[i][GESTURES::PINCH] = GESTURE_STATES::STOP;
		}

		/**
		* Grab state machine
		*/
		float grabThreshold = Settings::getInstance().GESTURES_PINCH_THRESHOLD;
		if (hand->visible && hand->grabStrength >= grabThreshold && gestures[i][GESTURES::GRAB] == GESTURE_STATES::STOP) {
			// Start
			gestures[i][GESTURES::GRAB] = GESTURE_STATES::START;
			gestureHandler->grab(gestures[i][GESTURES::GRAB], (HANDS)i, skeletonHands);
		}
		else if (hand->visible && hand->grabStrength >= grabThreshold && (gestures[i][GESTURES::GRAB] == GESTURE_STATES::START || gestures[i][GESTURES::GRAB] == GESTURE_STATES::UPDATE)) {
			// Update
			gestures[i][GESTURES::GRAB] = GESTURE_STATES::UPDATE;
			gestureHandler->grab(gestures[i][GESTURES::GRAB], (HANDS)i, skeletonHands);
		}
		else if (hand->visible && hand->grabStrength < grabThreshold) {
			// Stop
			gestures[i][GESTURES::GRAB] = GESTURE_STATES::STOP;
			gestureHandler->grab(gestures[i][GESTURES::GRAB], (HANDS)i, skeletonHands);
		}

		/**
		* Rotation gesture state machine
		*/
		Vector3f sphereCenter = mesh->getBoundingBox().getCenter();
		float sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * 0.5f;
		Vector3f midPoint = (hand->palm.position + hand->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
		Vector3f p = midPoint - sphereCenter;
		bool handInsideSphere = powf((p.x()), 2.f) + powf((p.y()), 2.f) + powf((p.z()), 2.f) <= powf((sphereRadius), 2.f);

		unsigned int extendedCount = 0;
		for (int j = 0; j < 5; j++) {
			if ((hand->visible && (hand->finger[j].extended || (!hand->finger[j].extended && hand->grabStrength <= 0.6f))))
				extendedCount++;
		}

		int otherHand = (i + 1) % 2;
		if (gestures[i][GESTURES::PINCH] == GESTURE_STATES::STOP && gestures[otherHand][GESTURES::ROTATION] == GESTURE_STATES::STOP && gestureZoom == GESTURE_STATES::STOP && hand->visible && handInsideSphere && extendedCount == 5 && gestures[i][GESTURES::ROTATION] == GESTURE_STATES::STOP) {
			// Start
			gestures[i][GESTURES::ROTATION] = GESTURE_STATES::START;
			gestureHandler->rotate(gestures[i][GESTURES::ROTATION], (HANDS)i, skeletonHands);
		}
		else if (gestures[i][GESTURES::PINCH] == GESTURE_STATES::STOP && gestures[otherHand][GESTURES::ROTATION] == GESTURE_STATES::STOP && gestureZoom == GESTURE_STATES::STOP && hand->visible && handInsideSphere && extendedCount == 5 && (gestures[i][GESTURES::ROTATION] == GESTURE_STATES::START || gestures[i][GESTURES::ROTATION] == GESTURE_STATES::UPDATE)) {
			// Update
			gestures[i][GESTURES::ROTATION] = GESTURE_STATES::UPDATE;
			gestureHandler->rotate(gestures[i][GESTURES::ROTATION], (HANDS)i, skeletonHands);
		}
		else if (gestures[i][GESTURES::PINCH] == GESTURE_STATES::STOP && gestures[otherHand][GESTURES::ROTATION] == GESTURE_STATES::STOP && (gestureZoom != GESTURE_STATES::STOP || !hand->visible || (handInsideSphere && extendedCount < 5) || (extendedCount == 5 && !handInsideSphere))) {
			// Stop
			gestures[i][GESTURES::ROTATION] = GESTURE_STATES::STOP;
			gestureHandler->rotate(gestures[i][GESTURES::ROTATION], (HANDS)i, skeletonHands);
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

void LeapListener::setMesh(std::shared_ptr<Mesh> &m) {
	mesh = m;
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
