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
	float confidence; ///< Confidence of hand position, ranges between [0, 1]
	Vector3f palmPosition; ///< Hands palm position
	Vector3f fingerPosition[5]; ///< Finger tip positions
	bool isRight; ///< Is it the right hand?
	struct mesh_s {
		Cube palm; ///< Mesh for the palm
		Cube finger[5]; ///< Mesh for all five fingers; 0 = Thumb, 4 = Pinky
	} mesh; ///< Composite mesh
	float pinchStrength; ///< Pinch strength
	float grabStrength; ///< Grab strength
};

VR_NAMESPACE_END
