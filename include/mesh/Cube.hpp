#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Cube
 *
 */
class Cube : public Mesh {
public:
	
	Cube();
	Cube(Vector3f &min, Vector3f &max);
	virtual ~Cube() = default;
	
};

VR_NAMESPACE_END
