#include "leap/SkeletonHand.hpp"

VR_NAMESPACE_BEGIN

SkeletonHand::SkeletonHand (bool _isRight)
	: isRight(_isRight), visible(false), id(-1), confidence(0.f), grabStrength(0.f), pinchStrength(0.0f) {

	palm.position = Vector3f(0.f, 0.f, 1000.f);

	mesh.palm.scale(0.025f, 0.025f, 0.025f);
	mesh.palm.translate(0.f, 0.f, 1000.f);
	
	for (int i = 0; i < 5; i++) {
		mesh.finger[i].scale(0.012f, 0.012f, 0.012f);
		mesh.finger[i].translate(0.f, 0.f, 1000.f);
		finger[i].position = Vector3f(0.f, 0.f, 1000.f);
	}

	handJointPosition = Vector3f(0.f, 0.f, 1000.f);
	mesh.handJoint.scale(0.012f, 0.012f, 0.012f);
	mesh.handJoint.translate(0.f, 0.f, 1000.f);

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < mesh.nrOfJoints; j++) {
			mesh.joints[i][j].scale(0.01f, 0.01f, 0.01f);
			mesh.joints[i][j].translate(0.f, 0.f, 1000.f);
			mesh.jointConnections[i][j] = Line(Vector3f(0.f, 0.f, 1000.f), Vector3f(0.f, 0.f, 1000.f));
		}
	}

	for (int j = 0; j < mesh.nrOfhandBones; j++)
		mesh.handBones[j] = Line(Vector3f(0.f, 0.f, 1000.f), Vector3f(0.f, 0.f, 1000.f));
	

}

void SkeletonHand::upload (std::shared_ptr<GLShader> &s) {
	shader = s;

	mesh.palm.upload(s);
	mesh.handJoint.upload(s);

	for (int i = 0; i < 5; i++)
		mesh.finger[i].upload(s);

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < mesh.nrOfJoints; j++)
			mesh.joints[i][j].upload(s);
}

void SkeletonHand::draw (const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	using namespace Leap;

	//mesh.palm.draw(viewMatrix, projectionMatrix);
	mesh.handJoint.draw(viewMatrix, projectionMatrix);

	Vector3f color = Settings::getInstance().MATERIAL_COLOR;
	shader->setUniform("materialColor", Settings::getInstance().MATERIAL_COLOR_ROTATION);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].draw(viewMatrix, projectionMatrix);
	shader->setUniform("materialColor", color);


	for (int i = 0; i < 5; i++)
		for (int j = 0; j < mesh.nrOfJoints; j++)
			mesh.joints[i][j].draw(viewMatrix, projectionMatrix);
	
	shader->setUniform("materialColor", Vector3f(0.8f, 0.8f, 0.8f));
	shader->setUniform("simpleColor", true);

	/**
	 * This not efficient at all to do it that way - but since a line consists
	 * only of two points its feasible.
	 */
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < mesh.nrOfJoints; j++) {
			if (j == mesh.nrOfJoints - 1)
				mesh.jointConnections[i][j].update(finger[i].jointPositions[j], finger[i].position);
			else
				mesh.jointConnections[i][j].update(finger[i].jointPositions[j], finger[i].jointPositions[j + 1]);

			mesh.jointConnections[i][j].upload(shader);
			mesh.jointConnections[i][j].draw(viewMatrix, projectionMatrix);
		}
	}

	// Close proximal and metacarpal
	mesh.handBones[0].update(finger[Finger::Type::TYPE_THUMB].jointPositions[1], finger[Finger::Type::TYPE_INDEX].jointPositions[0]);
	mesh.handBones[1].update(finger[Finger::Type::TYPE_INDEX].jointPositions[0], finger[Finger::Type::TYPE_MIDDLE].jointPositions[0]);
	mesh.handBones[2].update(finger[Finger::Type::TYPE_MIDDLE].jointPositions[0], finger[Finger::Type::TYPE_RING].jointPositions[0]);
	mesh.handBones[3].update(finger[Finger::Type::TYPE_RING].jointPositions[0], finger[Finger::Type::TYPE_PINKY].jointPositions[0]);
	mesh.handBones[4].update(finger[Finger::Type::TYPE_PINKY].jointPositions[0], handJointPosition);
	mesh.handBones[5].update(finger[Finger::Type::TYPE_THUMB].jointPositions[0], handJointPosition);

	
	for (int i = 0; i < mesh.nrOfhandBones; i++) {
		mesh.handBones[i].upload(shader);
		mesh.handBones[i].draw(viewMatrix, projectionMatrix);
	}

	shader->setUniform("materialColor", color);
	shader->setUniform("simpleColor", false);
}

void SkeletonHand::translate(float x, float y, float z) {
	mesh.palm.translate(x, y, z);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].translate(x, y, z);

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < mesh.nrOfJoints; j++)
			mesh.joints[i][j].translate(x, y, z);
}

bool SkeletonHand::containsBBox(const BoundingBox3f &b) {
	if (b.contains(palm.position))
		return true;

	if (b.contains(handJointPosition))
		return true;
	
	for (int i = 0; i < 5; i++) {
		if (b.contains(finger[i].position))
			return true;
		for (int j = 0; j < mesh.nrOfJoints; j++)
				if (b.contains(finger[i].jointPositions[j]))
				return true;
	}

	return false;
}

VR_NAMESPACE_END
