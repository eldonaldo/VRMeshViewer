#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

Mesh::Mesh ()
	: modelMatrix(Matrix4f::Identity()), normalMatrix(Matrix3f::Identity()) {

}

void Mesh::setModelMatrix (const Matrix4f &modelMatrix) {
	this->modelMatrix = modelMatrix;

	// Calculate normal matrix for normal transformation
	Matrix3f tmp = modelMatrix.topLeftCorner<3, 3>();
	Matrix3f inv = tmp.inverse();
	normalMatrix = inv.transpose();
}

VR_NAMESPACE_END
