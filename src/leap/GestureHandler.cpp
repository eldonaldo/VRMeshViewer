#include "leap/GestureHandler.hpp"

VR_NAMESPACE_BEGIN

GestureHandler::GestureHandler()
	: viewer(nullptr) {

}

void GestureHandler::pinch (GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand> (&hands)[2]) {
	Vector3f &position = hands[hand]->palmPosition;
	cout << "State: " << gestureStateName(state) << ", Hand: " << handName(hand) <<
			", [" << position.x() << ", " << position.y() << ", " << position.z() << "]" << endl;
}

void GestureHandler::setViewer (Viewer *v) {
	viewer = v;
}

VR_NAMESPACE_END
