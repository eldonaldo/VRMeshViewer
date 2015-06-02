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
	static Vector3f diff(0.f, 0.f, 0.f);

	switch (state) {
		case GESTURE_STATES::START: {
			BoundingBox3f bbox = mesh->getBoundingBox();
			diff = bbox.getCenter() - hands[hand]->finger[Finger::Type::TYPE_INDEX].position;
			break;
		}

		case GESTURE_STATES::UPDATE: {
			// Compute translation only if middle up to pinky are extended and thumb and index are near to each other (approx. 3cm)
			if (extended && distance <= threshold) {

				if (!Settings::getInstance().GESTURES_RELATIVE_TRANSLATE)
					// Translate absolute
					viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), hands[hand]->finger[Finger::Type::TYPE_INDEX].position);
				else
					// Translate relative
					viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), hands[hand]->finger[Finger::Type::TYPE_INDEX].position + diff);
			}

			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			diff = Vector3f(0.f, 0.f, 0.f);
			break;
		}
	}
}

void GestureHandler::rotate (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &right = hands[0], &left = hands[1];
	float distance = (right->palm.position - left->palm.position).norm();
	float dotProd = right->palm.normal.normalized().dot(left->palm.normal.normalized());

	switch (state) {
		case GESTURE_STATES::START: {
			break;
		}

		case GESTURE_STATES::UPDATE: {

			cout << "RIGHT - Roll: " << right->palm.roll << ", Pitch: " << right->palm.pitch << ", Yaw: " << right->palm.yaw << endl;
			//cout << "LEFT - Roll: " << left->palm.roll << ", Pitch: " << left->palm.pitch << ", Yaw: " << left->palm.yaw << endl;

			Eigen::AngleAxis<float> rollAngle(right->palm.roll, Eigen::Vector3f::UnitZ());
			Eigen::AngleAxis<float> yawAngle(right->palm.yaw, Eigen::Vector3f::UnitY());
			Eigen::AngleAxis<float> pitchAngle(right->palm.pitch, Eigen::Vector3f::UnitX());

			Eigen::Quaternion<float> q = rollAngle * yawAngle * pitchAngle;
				

			Matrix4f R = Matrix4f::Identity();
			R.block<3, 3>(0, 0) = q.toRotationMatrix();


			// Only if hands point together
			//if (dotProd <= -0.8f) {
				Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
				viewer->getRotationMatrix() = R;
				//cout << R << "\n" << endl;
			//}
			//else {
			//	Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.f, 0.f);
			//}
			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			break;
		}
	}

	scale(state, hands);
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

		case GESTURE_STATES::START:
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			lastDistance = distance;
			break;
		}
	}
}

void GestureHandler::grab(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	switch (state) {
		case GESTURE_STATES::START: {
			
			break;
		}

		case GESTURE_STATES::UPDATE: {
			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			break;
		}
	}
}

void GestureHandler::swipe(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::SwipeGesture &swipe) {
	/*switch (state) {
		case GESTURE_STATES::START: {
			break;
		}

		case GESTURE_STATES::UPDATE: {

			Leap::Vector v;
			Vector3f direction = pointerHand->finger[Finger::Type::TYPE_INDEX].direction.normalized();
			v.x = direction.x();
			v.y = direction.y();
			v.z = direction.z();
					
			Leap::Matrix T = Leap::Matrix(v, 1.f * DEG_TO_RAD);
			//Leap::Matrix T = Leap::Matrix(v.normalized(), 1.f * DEG_TO_RAD);
			Matrix4f M = Eigen::Map<Matrix4f>((float *)T.toArray4x4());
			viewer->getRotationMatrix() = viewer->getRotationMatrix()* M;
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default:
			Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			break;
	}*/
}

void GestureHandler::screenTap(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::ScreenTapGesture &tap) {

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
