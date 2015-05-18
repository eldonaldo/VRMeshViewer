#include "leap/GestureHandler.hpp"

using namespace Leap;

VR_NAMESPACE_BEGIN

GestureHandler::GestureHandler()
	 : viewer(nullptr) {

}

void GestureHandler::pinch (GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand> (&hands)[2]) {
	Vector3f &position = hands[hand]->finger[Finger::Type::TYPE_INDEX].position;
	
	//cout << "State: " << gestureStateName(state) << ", Hand: " << handName(hand) <<
	//		", [" << position.x() << ", " << position.y() << ", " << position.z() << "]" << endl;


	// Normalized x to [0, 1]
	float maxX = fabs(position.z() + Settings::getInstance().CAMERA_OFFSET.z()) * tanf(degToRad(75.f / 2.f));
	float normalizedX = 1.f - clamp(fabs((position.x() - maxX) / (2.f * maxX)));

	// Normalized y to [0, 1]
	float maxY = fabs(position.z() + Settings::getInstance().CAMERA_OFFSET.z()) * tanf(degToRad(75.f / 2.f));
	float normalizedY = clamp(fabs((position.y() - maxY) / (2.f * maxY)));

	//cout << "x: " << normalizedX << ", y: " << normalizedY << "" << endl;

	//switch (state) {
	//	case GESTURE_STATES::START:
	//		viewer->getArcball().button(viewer->getLastPos(), true);
	//		break;

	//	case GESTURE_STATES::UPDATE:
	//		viewer->setLastPos(Vector2i(int(normalizedX * viewer->width), int(normalizedY * viewer->height)));
	//		viewer->getArcball().motion(viewer->getLastPos());
	//		break;
	//	
	//	default:
	//	case GESTURE_STATES::STOP:
	//	case GESTURE_STATES::INVALID:
	//		viewer->getArcball().button(viewer->getLastPos(), false);
	//		break;
	//}
}

void GestureHandler::grab(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {

	Quaternionf q;
	q = Eigen::AngleAxis<float>(hands[hand]->palm.roll, hands[hand]->palm.normal)
	 * Eigen::AngleAxis<float>(hands[hand]->palm.yaw, hands[hand]->palm.direction)
	 * Eigen::AngleAxis<float>(hands[hand]->palm.pitch, hands[hand]->palm.direction);

	Matrix4f R = Matrix4f::Identity();
	R.block<3, 3>(0, 0) = q.toRotationMatrix();

	switch (state) {
		case GESTURE_STATES::START:
			break;

		case GESTURE_STATES::UPDATE:
			//viewer->getRotationMatrix() = R;
			break;

		default:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			break;
	}
}

void GestureHandler::zoom(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &right = hands[0], &left = hands[1];

	// From m -> mm to improve accurancy
	float distance = (right->palm.position * 1000.f - left->palm.position * 1000.f).squaredNorm();

	switch (state) {
		case GESTURE_STATES::UPDATE: {
			// Only if sqrt. distance is bigger than 2cm in change
			if (fabs(distance - lastDistance) / 1000.f >= 2.f) {
				float factor = 0.005f * (distance < lastDistance ? -1.f : 1.f);
				viewer->getScaleMatrix() = scale(viewer->getScaleMatrix(), factor);
				lastDistance = distance;
			}

			break;
		}

		default:
		case GESTURE_STATES::START:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			lastDistance = distance;
			break;
	}
}

void GestureHandler::swipe (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::SwipeGesture &swipe) {
	assert(swipe.hands().count() == 1);

	// Determine which hand does what
	auto &pointerHand = hands[HANDS::LEFT];
	if (swipe.hands()[0].isLeft())
		pointerHand = hands[HANDS::RIGHT];

	Quaternionf q;
	q = Eigen::AngleAxis<float>(swipe.duration() * DEG_TO_RAD, pointerHand->finger[Finger::Type::TYPE_INDEX].direction.normalized());

	Matrix4f R = Matrix4f::Identity();
	R.block<3, 3>(0, 0) = q.toRotationMatrix();

	switch (state) {
		case GESTURE_STATES::START:
			break;

		case GESTURE_STATES::UPDATE:
			//viewer->getRotationMatrix() = R;
			break;

		default:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			break;
	}
}

void GestureHandler::setViewer (Viewer *v) {
	viewer = v;
}

VR_NAMESPACE_END
