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

				// Compute translation only if middle up to pinky are extended and thumb and index are near to each other (approx. 3cm)
				if (distance <= threshold) {

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
				break;
		}
	} else {
		//// TODO: Remove
		//// Normalized vector to [0, 1]^2
		//Vector3f &position = hands[hand]->finger[Finger::Type::TYPE_INDEX].position;
		//Vector2f normalizedPosition = GestureHandler::normalize(position);
	
		//// Arcball 
		//float x = normalizedPosition.x() * (viewer->width / 2);
		//float y = normalizedPosition.y() * viewer->height;

		////cout << "x: " << x << ", y: " << y << "" << endl;

		//switch (state) {
		//	case GESTURE_STATES::START: {
		//		viewer->getArcball().button(viewer->getLastPos(), true);
		//		break;
		//	}

		//	case GESTURE_STATES::UPDATE: {
		//		// Compute translation only if middle up to pinky are extended and thumb and index are near to each other (approx. 3cm)
		//									 if (distance <= distance) {
		//			viewer->setLastPos(Vector2i(int(x), int(y)));
		//			viewer->getArcball().motion(viewer->getLastPos());
		//		}
		//		break;
		//	}

		//	default:
		//	case GESTURE_STATES::STOP:
		//	case GESTURE_STATES::INVALID:
		//		viewer->getArcball().button(viewer->getLastPos(), false);
		//		break;
		//}
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
	/*switch (state) {
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
	}*/
}

void GestureHandler::screenTap (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::ScreenTapGesture &tap) {

}

void GestureHandler::swipe(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::SwipeGesture &swipe) {
	//assert(swipe.hands().count() == 1);

	//static Matrix4f M(Matrix4f::Identity());
	//switch (state) {
	//	case GESTURE_STATES::START: {
	//		Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.f, 0.8f);
	//		Leap::Vector v;
	//		v.x = swipe.direction.x();
	//		v.y = swipe.direction.y();
	//		v.z = swipe.direction.z();
	//		
	//		Leap::Matrix T = Leap::Matrix(Leap::Vector(0.f, 1.f, 0.f), 1.f * DEG_TO_RAD);
	//		//Leap::Matrix T = Leap::Matrix(v.normalized(), 1.f * DEG_TO_RAD);
	//		M = Eigen::Map<Matrix4f>((float *)T.toArray4x4());
	//		cout << v << "\n" << endl;
	//		break;
	//	}

	//	case GESTURE_STATES::UPDATE: {
	//		Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);

	//		viewer->getRotationMatrix() = viewer->getRotationMatrix()* M;
	//		break;
	//	}

	//	default:
	//	case GESTURE_STATES::STOP:
	//	case GESTURE_STATES::INVALID:
	//		Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
	//		break;
	//}

	// Determine which hand does what
	//auto &pointerHand = hands[HANDS::LEFT];
	//if (swipe.hands()[0].isLeft())
	//	pointerHand = hands[HANDS::RIGHT];


	//switch (state) {
	//	case GESTURE_STATES::START:
	//		break;

	//	case GESTURE_STATES::UPDATE: {

	//	Leap::Vector v;
	//	Vector3f direction = pointerHand->finger[Finger::Type::TYPE_INDEX].direction.normalized();
	//	v.x = direction.x();
	//	v.y = direction.y();
	//	v.z = direction.z();
	//				
	//	Leap::Matrix T = Leap::Matrix(v, 1.f * DEG_TO_RAD);
	//	//Leap::Matrix T = Leap::Matrix(v.normalized(), 1.f * DEG_TO_RAD);
	//	Matrix4f M = Eigen::Map<Matrix4f>((float *)T.toArray4x4());
	//	viewer->getRotationMatrix() = viewer->getRotationMatrix()* M;
	//		Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
	//		break;
	//	}

	//	default:
	//	case GESTURE_STATES::STOP:
	//	case GESTURE_STATES::INVALID:
	//		Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
	//		break;
	//}
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
