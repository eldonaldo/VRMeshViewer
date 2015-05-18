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
	* @brief Swipe gesture
	*/
	virtual void swipe (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2], Leap::SwipeGesture &swipe);

	/**
	* @brief Sets the pointer to the Viewer
	*/
	void setViewer (Viewer *v);

	/**
	* @brief Sets the pointer to the mesh
	*/
	void setMesh (std::shared_ptr<Mesh> &m);

private:

	Viewer *viewer; ///< Viewer
	std::shared_ptr<Mesh> mesh; ///< Mesh
	float lastDistance = 0.f; ///< Last known sclaing distance in milimeter
};

VR_NAMESPACE_END
