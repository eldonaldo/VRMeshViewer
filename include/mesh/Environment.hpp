#pragma once

#include "common.hpp"
#include "mesh/WavefrontObj.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Environment sphere
 *
 */
class Environment : public WavefrontOBJ {
public:
	
	Environment(bool invertNormals = false);
	virtual ~Environment() = default;

	GLuint getVAO() {
		return vao;
	}
};

VR_NAMESPACE_END
