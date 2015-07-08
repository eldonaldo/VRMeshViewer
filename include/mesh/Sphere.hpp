#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Sphere
 *
 */
class Sphere : public Mesh {
public:
	
	Sphere();
	Sphere(float radius, unsigned int rings, unsigned int sectors, bool invertNormals = false);
	virtual ~Sphere() = default;
	float radius;

	GLuint getVAO() {
		return vao;
	}

protected:

	Vector2f getUV(const Vector3f &v);
};

VR_NAMESPACE_END
