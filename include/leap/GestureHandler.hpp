#pragma once

#include "common.hpp"
#include "Viewer.hpp"
#include "leap/SkeletonHand.hpp"

VR_NAMESPACE_BEGIN

/**
 * Gesture handling
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
	* @brief Zoom gesture
	*/
	virtual void zoom (GESTURE_STATES state, std::shared_ptr<SkeletonHand>(&hands)[2]);

	/**
	* @brief Sets the pointer to the Viewer
	*/
	void setViewer (Viewer *v);

private:

	Viewer *viewer; ///< Viewer
	float lastDistance = 0.f; ///< Last known sclaing distance in milimeter
};

VR_NAMESPACE_END
