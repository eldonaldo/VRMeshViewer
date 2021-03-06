#pragma once

#include "common.hpp"
#include "mesh/Cube.hpp"
#include "mesh/Sphere.hpp"
#include "mesh/Line.hpp"
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

	/**
	* @brief BBox containing check
	*/
	bool containsBBox(const BoundingBox3f &b);

public:

	int id; ///< Leap hand id
	bool isRight; ///< Is it the right hand?
	bool visible; ///< Is this hand in the FOV?

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
		Vector3f velocity;
		float pitch, yaw, roll;
	} palm;
	
	///< Fingers
	struct finger_t {
		Vector3f position; ///< Finger tip positions
		Vector3f direction; ///< Finger direction
		Vector3f jointPositions[3]; ///< Finger joint positions
		bool extended; ///< The finger extended or not?
	} finger[5];
	Vector3f handJointPosition; ///< Closing hand

	///< Composite mesh
	struct mesh_t {
		Sphere palm; ///< Mesh for the palm
		Sphere finger[5]; ///< Mesh for all five fingers; 0 = Thumb, 4 = Pinky
		Sphere joints[5][3]; ///< Finger joints
		Sphere handJoint; ///< Closing hand
		Line jointConnections[5][3]; ///< Joint connections
		Line handBones[6]; ///< Closing hand bones
		int nrOfJoints = 3;
		int nrOfhandBones = 6;
	} mesh; ///< Composite mesh

	std::shared_ptr<GLShader> shader; ///< Bounded shader
};

VR_NAMESPACE_END
