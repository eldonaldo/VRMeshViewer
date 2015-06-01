#include "leap/GestureHandler.hpp"

using namespace Leap;

VR_NAMESPACE_BEGIN

GestureHandler::GestureHandler()
	 : viewer(nullptr) {

}

void GestureHandler::pinch (GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand> (&hands)[2]) {
	bool extended = hands[hand]->finger[Finger::Type::TYPE_MIDDLE].extended && 
					hands[hand]->finger[Finger::Type::TYPE_RING].extended && 
					hands[hand]->finger[Finger::Type::TYPE_PINKY].extended;

	float distance = (hands[hand]->finger[Finger::Type::TYPE_INDEX].position - hands[hand]->finger[Finger::Type::TYPE_THUMB].position).norm();
	float threshold = 0.035f;

	if (extended) {
		static Vector3f diff(0.f, 0.f, 0.f);

		switch (state) {
		case GESTURE_STATES::START: {
			BoundingBox3f bbox = mesh->getBoundingBox();
			diff = bbox.getCenter() - hands[hand]->finger[Finger::Type::TYPE_INDEX].position;
			break;
		}

		case GESTURE_STATES::UPDATE: {
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
			float distance = (hands[hand]->finger[Finger::Type::TYPE_INDEX].position - hands[hand]->finger[Finger::Type::TYPE_THUMB].position).norm();
			
			// Compute translation only if middle up to pinky are extended and thumb and index are near to each other (approx. 3cm)
			if (hands[hand]->finger[Finger::Type::TYPE_MIDDLE].extended &&
				hands[hand]->finger[Finger::Type::TYPE_RING].extended && 
				hands[hand]->finger[Finger::Type::TYPE_PINKY].extended &&
				distance <= 0.035f) {

				if (!Settings::getInstance().GESTURES_RELATIVE_TRANSLATE)
					// Translate absolute
					viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), hands[hand]->finger[Finger::Type::TYPE_INDEX].position);
				else
					// Translate relative
					viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), hands[hand]->finger[Finger::Type::TYPE_INDEX].position + diff);
			}

			break;
		}

		default:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			diff = Vector3f(0.f, 0.f, 0.f);
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			break;
	}
}

void GestureHandler::grab(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {

	switch (state) {
		case GESTURE_STATES::START:
			break;

		case GESTURE_STATES::UPDATE:
			break;

		default:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			break;
	}
}

void GestureHandler::scale(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &right = hands[0], &left = hands[1];

	float distance = (right->palm.position - left->palm.position).norm();
	float dotProd = right->palm.normal.normalized().dot(left->palm.normal.normalized());

	switch (state) {
		case GESTURE_STATES::UPDATE: {
			// Only if distance is bigger than 2 cm in change and hands point together (dot product)
			if (dotProd <= -0.8f && fabs(distance - lastDistance) >= 0.02f) {	
				// We want that the bounding box fits our hands when scaling
				BoundingBox3f bbox = mesh->getBoundingBox();
				float diag = (bbox.max - bbox.min).norm();
				float factor = (distance / diag);

				// Compute scaling matrix
				viewer->getScaleMatrix() = VR_NS::scale(viewer->getScaleMatrix(), factor);
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

void GestureHandler::grab(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	switch (state) {
		case GESTURE_STATES::START: {
			
			break;
		}

		case GESTURE_STATES::UPDATE: {
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.f, 0.f);
			break;
		}

		default:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			break;
	}
}

void GestureHandler::screenTap (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::ScreenTapGesture &tap) {

}

void GestureHandler::swipe (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Swipe &swipe) {
	assert(swipe.hands().count() == 1);

	switch (state) {
		case GESTURE_STATES::START:
			break;

		case GESTURE_STATES::UPDATE: {
			//Quaternionf q;
			//cout << swipe.duration() << endl;
			//q = Eigen::AngleAxis<float>((swipe.duration() / 10000.f) * DEG_TO_RAD, pointerHand->finger[Finger::Type::TYPE_INDEX].direction.normalized());

			//Matrix4f R = Matrix4f::Identity();
			//R.block<3, 3>(0, 0) = q.toRotationMatrix();
			//viewer->getRotationMatrix() = R;

			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
			Leap::Matrix T = Leap::Matrix(swipe.direction(), 1.23);
			Matrix4f M = Eigen::Map<Matrix4f>((float *)T.toArray4x4());
			cout << M << "\n" << endl;
			viewer->getRotationMatrix() = M;
			break;
		}

		default:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			break;
	}
}

Vector2f GestureHandler::normalize(const Vector3f &v) {
	// Normalized x to [0, 1]
	float maxX = fabs(v.z() + Settings::getInstance().CAMERA_OFFSET.z()) * tanf(degToRad(75.f / 2.f));
	float normalizedX = 1.f - clamp(fabs((v.x() - maxX) / (2.f * maxX)));

	// Normalized y to [0, 1]
	float maxY = fabs(v.z() + Settings::getInstance().CAMERA_OFFSET.z()) * tanf(degToRad(75.f / 2.f));
	float normalizedY = clamp(fabs((v.y() - maxY) / (2.f * maxY)));

	return Vector2f(normalizedX, normalizedY);
}

void GestureHandler::setViewer (Viewer *v) {
	viewer = v;
}

void GestureHandler::setMesh (std::shared_ptr<Mesh> &m) {
	mesh = m;
}

VR_NAMESPACE_END
