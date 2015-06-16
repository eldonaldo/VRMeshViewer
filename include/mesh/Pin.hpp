#pragma once

#include "common.hpp"
#include "mesh/WavefrontObj.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Pin
 *
 * Objet to annotate meshes.
 *
 */
class Pin : public WavefrontOBJ {
public:
	
	Pin(Vector3f &pos);
	virtual ~Pin() = default;

	/**
	* @brief Return pin position
	*/
	Vector3f &getPosition();
	void setColor(Vector3f &c);

	virtual void draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);
	virtual Matrix4f getModelMatrix();
protected:

	Vector3f color;
	Vector3f position; ///< Pin position in world coordinates
	
};

VR_NAMESPACE_END
