#pragma once

#include "common.hpp"
#include <sstream>
#include "WavefrontObj.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Cube
 *
 */
class Cube : public WavefrontOBJ {
public:
	
	Cube();
	Cube(Vector3f min, Vector3f max);
	virtual ~Cube() = default;
	
};

VR_NAMESPACE_END
