#pragma once

#include "common.hpp"
#include "mesh/Cube.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Pin
 *
 * Objet to annotate meshes.
 *
 */
class Pin : public Cube {
public:
	
	Pin(Vector3f &pos);
	virtual ~Pin() = default;

	/**
	* @brief Return pin position
	*/
	Vector3f &getPosition();

	virtual Matrix4f getModelMatrix();

protected:

	Vector3f position; ///< Pin position in world coordinates
	
};

VR_NAMESPACE_END
