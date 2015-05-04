#include "mesh/Cube.hpp"

VR_NAMESPACE_BEGIN

Cube::Cube() : Mesh() {
	Point3f min(-1.f, -1.f, -1.f);
	Point3f max(1.f, 1.f, 1.f);

	MatrixXf V(3, 8);
	V.col(0) = Vector3f(min.x(), min.y(), min.z());
	V.col(1) = Vector3f(min.x(), min.y(), max.z());
	V.col(2) = Vector3f(max.x(), min.y(), max.z());
	V.col(3) = Vector3f(max.x(), min.y(), min.z());
	V.col(4) = Vector3f(max.x(), max.y(), max.z());
	V.col(5) = Vector3f(max.x(), max.y(), min.z());
	V.col(6) = Vector3f(min.x(), max.y(), min.z());
	V.col(7) = Vector3f(min.x(), max.y(), max.z());

	MatrixXu F(3, 12);
	F.col(0) = Vector3ui(0, 1, 7);
	F.col(1) = Vector3ui(0, 7, 6);
	F.col(2) = Vector3ui(0, 1, 2);
	F.col(3) = Vector3ui(0, 2, 3);
	F.col(4) = Vector3ui(2, 3, 4);
	F.col(5) = Vector3ui(3, 4, 5);
	F.col(6) = Vector3ui(4, 6, 7);
	F.col(7) = Vector3ui(4, 5, 6);
	F.col(8) = Vector3ui(1, 2, 4);
	F.col(9) = Vector3ui(1, 4, 7);
	F.col(10) = Vector3ui(0, 3, 6);
	F.col(11) = Vector3ui(3, 5, 6);
	
	m_F = F;
	m_V = V;

	scale(0.04f, 0.02f, 0.04f);
}

VR_NAMESPACE_END
