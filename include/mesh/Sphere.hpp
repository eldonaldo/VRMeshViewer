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
	
	Sphere(float radius, unsigned int rings, unsigned int sectors);
	virtual ~Sphere() = default;
	
};

VR_NAMESPACE_END
