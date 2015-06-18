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
	
	Pin(Vector3f &pos, Vector3f &n, Matrix3f &nm);
	virtual ~Pin() = default;

	/**
	* @brief Return pin position
	*/
	Vector3f &getPosition();
	void setColor(Vector3f &c);

	virtual void draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);
	virtual Matrix4f getModelMatrix();
	std::string serialize();

protected:

	void calculateLocalRotation(Matrix3f &nm);

protected:

	Matrix4f localRotation;
	Vector3f color;
	Vector3f position; ///< Pin position in local coordinates
	Vector3f normal; ///< Normal coordinates of the mesh position
};

VR_NAMESPACE_END
