#include "leap/SkeletonHand.hpp"

VR_NAMESPACE_BEGIN

SkeletonHand::SkeletonHand (bool _isRight)
	: isRight(_isRight), id(-1), confidence(0.f), grabStrength(0.f), pinchStrength(0.0f) {

	palm.position = Vector3f(0.f, 0.f, 0.f);
	mesh.palm.scale(0.03f, 0.03f, 0.01f);
	
	for (int i = 0; i < 5; i++) {
		mesh.finger[i].scale(0.012f, 0.012f, 0.012f);
		finger[i].position = Vector3f(0.f, 0.f, 0.f);
	}

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < mesh.nrOfJoints; j++) {
			mesh.joints[i][j].scale(0.01f, 0.01f, 0.01f);
			mesh.joints[i][j].translate(0.f, 0.f, 1000.f);
			mesh.jointConnections[i][j].translate(0.f, 0.f, 1000.f);
		}
	}
}

void SkeletonHand::upload (std::shared_ptr<GLShader> &s) {
	shader = s;

	mesh.palm.upload(s);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].upload(s);

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < mesh.nrOfJoints; j++)
			mesh.joints[i][j].upload(s);
}

void SkeletonHand::draw (const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	mesh.palm.draw(viewMatrix, projectionMatrix);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].draw(viewMatrix, projectionMatrix);

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < mesh.nrOfJoints; j++)
			mesh.joints[i][j].draw(viewMatrix, projectionMatrix);

	/**
	 * This not efficient at all to do it that way - but since a line consists
	 * only of two points its feasible.
	 */
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < mesh.nrOfJoints; j++) {
			mesh.jointConnections[i][j].releaseBuffers();
			if (j == 0)
				mesh.jointConnections[i][j] = Line(finger[i].position, finger[i].jointPositions[j]);
			else
				mesh.jointConnections[i][j] = Line(finger[i].jointPositions[j - 1], finger[i].jointPositions[j]);

			mesh.jointConnections[i][j].upload(shader);

			shader->setUniform("bbox", true);
			mesh.jointConnections[i][j].draw(viewMatrix, projectionMatrix);
			shader->setUniform("bbox", false);
		}
	}
}

void SkeletonHand::translate(float x, float y, float z) {
	mesh.palm.translate(x, y, z);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].translate(x, y, z);

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < mesh.nrOfJoints; j++)
			mesh.joints[i][j].translate(x, y, z);
}

VR_NAMESPACE_END
