#include "mesh/Sphere.hpp"

VR_NAMESPACE_BEGIN

Sphere::Sphere() : Sphere(1.f, 24, 24) {

}

Sphere::Sphere(float radius, unsigned int rings, unsigned int sectors) : Mesh(), radius(radius) {
	float const R = 1.f / (float) (rings - 1);
	float const S = 1.f / (float) (sectors - 1);

	m_V.resize(3, rings * sectors);
	m_N.resize(3, rings * sectors);

	unsigned int i = 0;
	for (unsigned int r = 0; r < rings; r++) {
		for (unsigned int s = 0; s < sectors; s++) {
			float y = sinf(-M_PI_2 + M_PI * r * R);
			float x = cosf(2 * M_PI * s * S) * sinf(M_PI * r * R);
			float z = sinf(2 * M_PI * s * S) * sinf(M_PI * r * R);

			m_V.col(i) = Vector3f(x * radius, y * radius, z * radius);
			m_N.col(i) = Vector3f(x, y, z);
			i++;
		}
	}

	i = -1;
	m_F.resize(3, rings * sectors * 2);
	for (unsigned int r = 0; r < rings - 1; r++) {
		for (unsigned int s = 0; s < sectors - 1; s++) {
			m_F.col(++i) = Vector3ui(r * sectors + (s + 1), r * sectors + s, (r + 1) * sectors + s);
			m_F.col(++i) = Vector3ui(r * sectors + (s + 1), (r + 1) * sectors + s, (r + 1) * sectors + (s + 1));
		}
	}
}

VR_NAMESPACE_END
