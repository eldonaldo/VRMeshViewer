#include "leap/SkeletonHand.hpp"

VR_NAMESPACE_BEGIN

SkeletonHand::SkeletonHand (bool _isRight)
	: isRight(_isRight), id(-1), confidence(0.f), grabStrength(0.f), pinchStrength(0.0f) {

	palm.position = Vector3f(0.f, 0.f, 1000.f);
	mesh.palm.scale(0.03f, 0.03f, 0.005f);
	
	for (int i = 0; i < 5; i++) {
		mesh.finger[i].scale(0.012f, 0.012f, 0.012f);
		finger[i].position = Vector3f(0.f, 0.f, 1000.f);
	}
}

void SkeletonHand::upload (std::shared_ptr<GLShader> &s) {
	mesh.palm.upload(s);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].upload(s);
}

void SkeletonHand::draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	mesh.palm.draw(viewMatrix, projectionMatrix);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].draw(viewMatrix, projectionMatrix);
}

void SkeletonHand::translate(float x, float y, float z) {
	mesh.palm.translate(x, y, z);
	for (int i = 0; i < 5; i++)
		mesh.finger[i].translate(x, y, z);
}

VR_NAMESPACE_END
