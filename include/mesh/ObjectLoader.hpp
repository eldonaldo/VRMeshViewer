#pragma once

#include "common.hpp"
#include "Mesh.hpp"
#include <fstream>

VR_NAMESPACE_BEGIN

/**
 *
 */
class ObjectLoader {
public:
	static std::shared_ptr<Mesh> loadOBJ (std::string path) throw ();
};

VR_NAMESPACE_END
