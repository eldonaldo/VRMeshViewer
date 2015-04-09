#pragma once

#include "common.hpp"
#include "Mesh.hpp"
#include <fstream>

VR_NAMESPACE_BEGIN

/**
 * @brief Basic mesh loader class
 */
class ObjectLoader {
public:

	/**
	 * @brief Loads a standard OBJ object
	 *
	 * @param path Filepath
	 * @return Shared pointer to a mesh object
	 */
	static std::shared_ptr<Mesh> loadOBJ (std::string path) throw ();
};

VR_NAMESPACE_END
