#pragma once

#include "common.hpp"
#include "mesh/WavefrontObj.hpp"
#include <sstream>

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
	void update(const Vector3f &min, const Vector3f &max);

	// Override to use better cache locality
	void upload(std::shared_ptr<GLShader> &s);
};

VR_NAMESPACE_END
