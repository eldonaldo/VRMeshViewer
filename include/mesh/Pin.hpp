#pragma once

#include "common.hpp"
#include "mesh/WavefrontObj.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Pin
 *
 * Object to annotate meshes.
 */
class Pin : public WavefrontOBJ {
public:
	
	/**
	* @brief Create pin from position, normal and normal matrix
	*/
	Pin(Vector3f &pos, Vector3f &n, Matrix3f &nm);
	virtual ~Pin() = default;

	/**
	* @brief Return pin position
	*/
	const Vector3f &getPosition() const;

	/**
	* @brief Set pin color
	*/
	void setColor(Vector3f &c);

	/**
	* @brief Overloaded draw
	*/
	virtual void draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);

	/**
	* @brief Overloaded model matrix
	*/
	virtual Matrix4f getModelMatrix();

	/**
	* @brief Serialize pin state
	*/
	std::string serialize();

	/**
	* @brief Overloaded equality operator
	*/
	bool operator==(const Pin &rhs) const {
		return vector3fAlmostEqual(position, rhs.getPosition());
	}

	/**
	* @brief Overloaded inequality operator
	*/
	bool operator!=(const Pin &rhs) const {
		return !operator==(rhs);
	}

protected:

	/**
	* @brief Calculate local pin rotation
	*/
	void calculateLocalRotation(Matrix3f &nm);

protected:

	Matrix4f localRotation; ///< Local rotation matrix to make perpendicular to surface
	Vector3f color; ///< Pin color
	Vector3f position; ///< Pin position in local coordinates
	Vector3f normal; ///< Normal coordinates of the mesh position
};

VR_NAMESPACE_END
