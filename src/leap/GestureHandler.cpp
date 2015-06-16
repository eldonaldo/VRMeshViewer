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
			diff = mesh->getBoundingBox().getCenter() - hands[hand]->finger[Finger::Type::TYPE_INDEX].position;
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
			break;
		}
	}
}

void GestureHandler::rotate(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	static Vector3f lastPos(0.f, 0.f, 0.f); /// Last hand position
	static Quaternionf incr = Quaternionf::Identity(), quat = Quaternionf::Identity(); /// Rotation quaternions

	// Calculate sphere
	Vector3f midPoint = (hands[hand]->palm.position + hands[hand]->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	
	static Vector3f center(0.f, 0.f, 0.f);
	static float radius = 0.f;

	switch (state) {
		case GESTURE_STATES::START: {
			Settings::getInstance().SHOW_SPHERE = true;
			center = mesh->getBoundingBox().getCenter();
			radius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * 0.5f;
			lastPos = projectOnSphere(midPoint, center, radius);
			incr = Quaternionf::Identity();
			break;
		}

		case GESTURE_STATES::UPDATE: {
			// Project points on sphere around bbox center
			midPoint = projectOnSphere(midPoint, center, radius);
			
			// Rotation axis and angle
			Vector3f axis = lastPos.cross(midPoint);
			float sa = std::sqrt(axis.dot(axis));
			float ca = lastPos.dot(midPoint);
			float angle = std::atan2(sa, ca);
			
			// Compute rotation using quats
			incr = Eigen::AngleAxisf(angle, axis.normalized());
			if (!std::isfinite(incr.norm()))
				incr = Quaternionf::Identity();

			// Construct rotation matrix
			Matrix4f result = Matrix4f::Identity();
			result.block<3, 3>(0, 0) = (incr * quat).toRotationMatrix();
			viewer->getRotationMatrix() = result;

 			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			Settings::getInstance().SHOW_SPHERE = false;
			lastPos = projectOnSphere(midPoint, center, radius);
			quat = (incr * quat).normalized();
			incr = Quaternionf::Identity();
			break;
		}
	}
}

void GestureHandler::annotate(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &h = hands[hand];
	static bool found = false;

	switch (state) {
		case GESTURE_STATES::START: {
			found = false;
			break;
		}

		case GESTURE_STATES::UPDATE: {
			if (!found) {
				std::shared_ptr<Mesh> mesh = viewer->getMesh();
				MatrixXf V = mesh->getVertexPositions();
				for (int i = 0; i < V.cols(); i++) {
					Vector4f vLocal(V.col(i).x(), V.col(i).y(), V.col(i).z(), 1.f);
					Vector3f vWorld = (mesh->getModelMatrix() * vLocal).head<3>();
					if ((vWorld - h->finger[Finger::Type::TYPE_INDEX].position).norm() <= 0.005f) {
						found = true;
						
						// Notify the viewer
						viewer->uploadAnnotation = true;
						viewer->annotationTarget = vLocal.head<3>();
						viewer->annotationNormal = mesh->getVertexNormals().col(i);
						break;
					}
				}
			}

			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			found = false;
			break;
		}
	}

}

void GestureHandler::scale(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &right = hands[0], &left = hands[1];
	float dotProd = right->palm.normal.normalized().dot(left->palm.normal.normalized());
	float distance = (right->palm.position - left->palm.position).norm();

	Vector3f midRight = (right->palm.position + right->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f midLeft = (left->palm.position + left->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f handSphereCenter = (midRight + midLeft) * 0.5f;

	switch (state) {
		case GESTURE_STATES::UPDATE: {
			// Only if distance is bigger than 2 cm in change and hands point together (dot product)
			if (dotProd <= -0.6f && fabs(distance - lastDistance) >= 0.01f) {
				// We want that the bounding box fits our hands when scaling
				BoundingBox3f bbox = mesh->getBoundingBox();
				float diag = (bbox.max - bbox.min).norm();
				float factor = (distance / diag);

				// Compute scaling matrix
				viewer->getScaleMatrix() = VR_NS::scale(viewer->getScaleMatrix(), factor);
				lastDistance = distance;

			}
			
			if (dotProd <= -0.6f)
				viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), handSphereCenter);

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

Vector3f GestureHandler::projectOnSphere(Vector3f &v, Vector3f &sphereCenter, float sphereRadius) {
	Vector3f r = v - sphereCenter;
	Vector3f projectedR = (sphereRadius / r.norm()) * r;
	return projectedR;
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
