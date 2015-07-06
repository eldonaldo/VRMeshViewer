#include "leap/GestureHandler.hpp"

using namespace Leap;

VR_NAMESPACE_BEGIN

GestureHandler::GestureHandler()
	 : viewer(nullptr) {

}

void GestureHandler::pinch (GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand> (&hands)[2]) {
	auto &h = hands[hand];
	static bool found = false;
	Vector3f avgPinchPos = (hands[hand]->finger[Finger::Type::TYPE_INDEX].position + hands[hand]->finger[Finger::Type::TYPE_THUMB].position) * 0.5f;

	switch (state) {
		case GESTURE_STATES::START: {
			found = false;
			break;
		}

		case GESTURE_STATES::UPDATE: {
			//Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
			// Convert world coordinates to local coordinates
			Matrix4f worldToLocal = mesh->getModelMatrix().inverse();
			Vector3f localTipPosition = (worldToLocal * Vector4f(avgPinchPos.x(), avgPinchPos.y(), avgPinchPos.z(), 1.f)).head(3);
			
			// Check if we hit any existing pin. If yes, remove it
			if (!found)
				found = viewer->deletePinIfHit(avgPinchPos);

			// Use the kdtree to search for the nearest point to the tip position to place a pin
			if (!found) {
				KDTree kdtree = mesh->getKDTree();

				// Perform search
				std::vector<uint32_t> results;
				kdtree.search(localTipPosition, Settings::getInstance().ANNOTATION_SEACH_RADIUS, results);

				// If there is a hit upload a pin
				if (!results.empty()) {

					// We take the first hit
					GenericKDTreeNode<Point3f, Point3f> kdtreeNode = kdtree[results[0]];

					// Notify the viewer
					viewer->uploadAnnotation = true;
					viewer->annotationTarget = kdtreeNode.getPosition();
					viewer->annotationNormal = kdtreeNode.getData();
					found = true;
				}
			}

			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			//Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			found = false;
			break;
		}
	}
}

void GestureHandler::rotate(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	static Vector3f lastPos(0.f, 0.f, 0.f);
	static Quaternionf quat = Quaternionf::Identity();
	Vector3f midPoint = (hands[hand]->palm.position + hands[hand]->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	
	static Vector3f center(0.f, 0.f, 0.f);
	static float radius = 0.f;

	switch (state) {
		case GESTURE_STATES::START: {
			//							Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.f, 0.f);
			Settings::getInstance().SHOW_SPHERE = true;
			center = mesh->getBoundingBox().getCenter();
			radius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_MEDIUM_SCALE;
			lastPos = projectOnSphere(midPoint, center, radius);
			quat = Quaternionf::Identity();
			break;
		}

		case GESTURE_STATES::UPDATE: {
			//							 Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.f, 0.f);
			// Calculate distance to displayed sphere for visual assistance
			float assistanceRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_VISUAL_SCALE;
			Vector3f spherePoint = projectToSphere(midPoint, center, assistanceRadius);
			float distance = (spherePoint - midPoint).norm();
			distance /= ((Settings::getInstance().SPHERE_VISUAL_SCALE - Settings::getInstance().SPHERE_SMALL_SCALE) * 0.5f);
			Settings::getInstance().MATERIAL_COLOR_ROTATION = Vector3f(distance, 1.f - distance, 0.f);

			// Project points on sphere around bbox center
			midPoint = projectOnSphere(midPoint, center, radius);

			// Rotation axis and angle
			Vector3f axis = lastPos.cross(midPoint);
			float angle = acosf(lastPos.dot(midPoint) / ((lastPos.norm() * midPoint.norm())));

			// Compute rotation using quats
			quat = Eigen::AngleAxisf(angle, axis.normalized());
			if (!std::isfinite(quat.norm()))
				quat = Quaternionf::Identity();

			// Construct rotation matrix
			Matrix4f result = Matrix4f::Identity();
			result.block<3, 3>(0, 0) = quat.toRotationMatrix();
			viewer->getRotationMatrix() = result * viewer->getRotationMatrix();

			// Reset tracking state
			lastPos = midPoint;
			quat = Quaternionf::Identity();

			// Need to send a new packet
			Settings::getInstance().NETWORK_NEW_DATA = true;
 			break;
		}

		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
			Settings::getInstance().MATERIAL_COLOR_ROTATION = Settings::getInstance().MATERIAL_COLOR;
			Settings::getInstance().SHOW_SPHERE = false;
			//		 Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
			break;
		}
	}
}

void GestureHandler::scale(GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]) {
	auto &right = hands[0], &left = hands[1];
	float distance = (right->palm.position - left->palm.position).norm();
	float dotProd = right->palm.normal.normalized().dot(left->palm.normal.normalized());
	float dotProdThreshold = -0.45f;

	Vector3f midRight = (right->palm.position + right->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f midLeft = (left->palm.position + left->finger[Finger::Type::TYPE_MIDDLE].position) * 0.5f;
	Vector3f handSphereCenter = (midRight + midLeft) * 0.5f;

	static float diagStart = 0.f;
	static Matrix4f initialScale = Matrix4f::Identity();

	switch (state) {
		case GESTURE_STATES::START: {
			// Hands need to point together
			if (dotProd <= dotProdThreshold) {
				//Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
				diagStart = (mesh->getBoundingBox().max - mesh->getBoundingBox().min).norm();
				initialScale = viewer->getScaleMatrix();
			}
		}

		case GESTURE_STATES::UPDATE: {
			// Hands need to point together
			//Settings::getInstance().MATERIAL_COLOR = Vector3f(0.f, 0.8f, 0.f);
			if (dotProd <= dotProdThreshold) {
				// We want that the bounding box fits our hands when scaling
				BoundingBox3f bbox = mesh->getBoundingBox();
				float diag = (bbox.max - bbox.min).norm();
				float factor = (distance / diag);

				// Compute scaling matrix
				viewer->getScaleMatrix() = VR_NS::scale(initialScale, (factor * diag) / diagStart);

				// Need to send a new packet
				Settings::getInstance().NETWORK_NEW_DATA = true;

				viewer->getTranslateMatrix() = VR_NS::translate(Matrix4f::Identity(), handSphereCenter);

				// Need to send a new packet
				Settings::getInstance().NETWORK_NEW_DATA = true;
			}

			break;
		}

		
		case GESTURE_STATES::STOP:
		case GESTURE_STATES::INVALID:
		default: {
					// Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
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

Vector3f GestureHandler::projectOnSphere(const Vector3f &v, const Vector3f &sphereCenter, float sphereRadius) {
	Vector3f r = v - sphereCenter;
	Vector3f projectedR = (sphereRadius / r.norm()) * r;
	return projectedR;
}

Vector3f GestureHandler::projectToSphere(const Vector3f &v, const Vector3f &sphereCenter, float sphereRadius) {
	Vector3f r = v - sphereCenter;
	Vector3f projectedR = (sphereRadius / r.norm()) * r;
	return projectedR + sphereCenter;
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
