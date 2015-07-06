#pragma once

#include "common.hpp"
#include "Viewer.hpp"
#include "leap/SkeletonHand.hpp"

VR_NAMESPACE_BEGIN

/**
 * Gesture handling.
 *
 * Handling is based on state machines.
 */
class GestureHandler {
public:
	/**
	 * @brief Default
	 */
	GestureHandler ();
	virtual ~GestureHandler () = default;

	/**
	 * @brief Pinch gesture
	 */
	virtual void pinch (GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand> (&hands)[2]);

	/**
	* @brief Grab gesture
	*/
	virtual void grab (GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]);

	/**
	* @brief Scale gesture
	*/
	virtual void scale (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]);

	/**
	* @brief Rotate gesture
	*/
	virtual void rotate(GESTURE_STATES state, HANDS hand, std::shared_ptr<SkeletonHand>(&hands)[2]);

	/**
	* @brief Sets the pointer to the Viewer
	*/
	void setViewer (Viewer *v);

	/**
	* @brief Sets the pointer to the mesh
	*/
	void setMesh (std::shared_ptr<Mesh> &m);

	/**
	* @brief Transform Leap -> Rift coordinates to normalized 2D coordinates [0, 1] x [0, 1]
	*
	* Top left = 0, bottom right = 1
	*/
	static Vector2f normalize (const Vector3f &v);

	// Project point on sphere
	Vector3f projectOnSphere(const Vector3f &v, const Vector3f &sphereCenter, float sphereRadius);

	// Project a point to the sphere sourrding that point
	Vector3f projectToSphere(const Vector3f &v, const Vector3f &sphereCenter, float sphereRadius);

private:

	Viewer *viewer; ///< Viewer
	std::shared_ptr<Mesh> mesh; ///< Mesh
};

VR_NAMESPACE_END
