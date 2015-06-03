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
	float dotProd = right->palm.normal.normalized().dot(left->palm.normal.normalized());

	float speedFactor = 1.5f; /// Rotation speed factor
	static Vector3f lastPosRight(0.f, 0.f, 0.f), lastPosLeft(0.f, 0.f, 0.f); /// Last hand positions
	static Quaternionf incr = Quaternionf::Identity(), quat = Quaternionf::Identity(); /// Rotation quaternions

	// Compute bounding sphere
	Vector3f midRight = (right->palm.position + right->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f midLeft = (left->palm.position + left->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	viewer->sphereCenter = (midRight + midLeft) * 0.5f;
	viewer->sphereRadius = (right->palm.position - left->palm.position).norm() * 0.5f;
	
	// Is the center of the bbox lay inside the sphere?
	Vector3f p = mesh->getBoundingBox().getCenter() - viewer->sphereCenter;
	bool insideSphere = powf((p.x()), 2.f) + powf((p.y()), 2.f) + powf((p.z()), 2.f) <= powf((viewer->sphereRadius), 2.f);

	switch (state) {
		case GESTURE_STATES::START: {
			lastPosRight = right->palm.position;
			lastPosLeft = left->palm.position;
			incr = Quaternionf::Identity();
			break;
		}

		case GESTURE_STATES::UPDATE: {
			// Hands must point together and only if object resides inside the avg. hand sphere
			if (dotProd <= -0.6f && insideSphere) {
				Vector3f posRight = right->palm.position;
				Vector3f posLeft = left->palm.position;
				
				// Rotation axis and angle for each hand
				Vector3f axisRight = lastPosRight.cross(posRight);
				Vector3f axisLeft = lastPosLeft.cross(posLeft);
				float saR = std::sqrt(axisRight.dot(axisRight)), caR = lastPosRight.dot(posRight), angleR = std::atan2(saR, caR);
				float saL = std::sqrt(axisLeft.dot(axisLeft)), caL = lastPosLeft.dot(posLeft), angleL = std::atan2(saL, caL);

				// Compute average angle and axis
				float angle = (angleR + angleL) * 0.5f;
				Vector3f axis = axisRight + axisLeft;
				angle *= speedFactor;

				// Consider pitch angle
				float anglePitch = -(right->palm.pitch + left->palm.pitch) * 0.5f;
				Vector3f axisPitch = (-right->palm.normal + left->palm.normal) * 0.5f;;
				anglePitch *= speedFactor;

				// Compute rotation using quats
				incr = Eigen::AngleAxisf(angle, axis.normalized()) * Eigen::AngleAxisf(anglePitch, axisPitch.normalized());
				if (!std::isfinite(incr.norm()))
					incr = Quaternionf::Identity();

				// Construct rotation matrix
				Matrix4f result = Matrix4f::Identity();
				result.block<3, 3>(0, 0) = (incr * quat).toRotationMatrix();
				viewer->getRotationMatrix() = result;



				viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), viewer->sphereCenter);
			}
			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			lastPosRight = right->palm.position;
			lastPosLeft = left->palm.position;
			quat = (incr * quat).normalized();
			incr = Quaternionf::Identity();
			break;
		}
	}
	
	if (dotProd <= -0.8f && insideSphere) {
		scale(state, hands);
	}
}

void GestureHandler::scale(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &right = hands[0], &left = hands[1];
	float distance = (right->palm.position - left->palm.position).norm();

	switch (state) {
		case GESTURE_STATES::UPDATE: {
			// Only if distance is bigger than 2 cm in change and hands point together (dot product)
			if (fabs(distance - lastDistance) >= 0.02f) {	
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
