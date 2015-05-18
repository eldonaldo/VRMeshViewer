#pragma once

#include "common.hpp"
#include "mesh/Cube.hpp"
#include "GLUtil.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief Leap Skeleton hand
 */
class SkeletonHand {
public:

	/**
	* @brief Default
	*/
	SkeletonHand (bool _isRight);
	virtual ~SkeletonHand () = default;

	/**
	 * @brief Upload the mesh (positions, normals, indices and uv) to the shader
	 */
	virtual void upload (std::shared_ptr<GLShader> &s);

	/**
	 * @brief Draw to the currently bounded shader
	 */
	virtual void draw (const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);

	/**
	 * @brief Translation for all fingers and palm
	 */
	void translate (float x, float y, float z);

public:

	int id; ///< Leap hand id
	bool isRight; ///< Is it the right hand?

	/// Estimates
	float confidence; ///< Confidence of hand position, ranges between [0, 1]
	float pinchStrength; ///< Pinch strength
	float grabStrength; ///< Grab strength
	
	///< Palm
	struct palm_t {
		Vector3f position; ///< Palm position
		Vector3f direction; ///< Palm direction
		Vector3f side; /// Palm side
		Vector3f normal; ///< Palm normal
		float pitch, yaw, roll;
	} palm;
	
	///< Fingers
	struct finger_t {
		Vector3f position; ///< Finger tip positions
		Vector3f direction; ///< Finger direction
		bool extended;
	} finger[5];

	///< Composite mesh
	struct mesh_t {
		Cube palm; ///< Mesh for the palm
		Cube finger[5]; ///< Mesh for all five fingers; 0 = Thumb, 4 = Pinky
	} mesh; ///< Composite mesh
};

VR_NAMESPACE_END
