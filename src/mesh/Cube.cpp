#include "mesh/Cube.hpp"

VR_NAMESPACE_BEGIN

Cube::Cube() : Mesh() {
	Point3f min(-1.f, -1.f, -1.f);
	Point3f max(1.f, 1.f, 1.f);
	Cube(min, max);
}

Cube::Cube(Vector3f &min, Vector3f &max) : Mesh() {
	// Face normals
	Vector3f A(0.f, -1.f, 0.f);
	Vector3f B(1.f, 0.f, 0.f);
	Vector3f C(0.f, 1.f, 0.f);
	Vector3f D(-1.f, 0.f, 0.f);
	Vector3f E(0.f, 0.f, 1.f);
	Vector3f F1(0.f, 0.f, -1.f);

	// Vertices
	MatrixXf V(3, 8);
	V.col(0) = Vector3f(min.x(), min.y(), min.z());
	V.col(1) = Vector3f(min.x(), min.y(), max.z());
	V.col(2) = Vector3f(max.x(), min.y(), max.z());
	V.col(3) = Vector3f(max.x(), min.y(), min.z());
	V.col(4) = Vector3f(max.x(), max.y(), max.z());
	V.col(5) = Vector3f(max.x(), max.y(), min.z());
	V.col(6) = Vector3f(min.x(), max.y(), min.z());
	V.col(7) = Vector3f(min.x(), max.y(), max.z());

	// Vertix normals
	MatrixXf N(3, 8);
	N.col(0) = A + D + F1;
	N.col(1) = A + E + D;
	N.col(2) = A + E + B;
	N.col(3) = A + B + F1;
	N.col(4) = B + E + C;
	N.col(5) = B + F1 + C;
	N.col(6) = F1 + D + C;
	N.col(7) = D + E + C;

	// Faces
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
	m_N = N;
	m_V = V;
}

VR_NAMESPACE_END
