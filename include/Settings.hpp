#pragma once

#include "Eigen/Core"
#include "Leap.h"

/// To avoid cycling includes
#if !defined(VR_NAMESPACE_BEGIN)
	#define VR_NAMESPACE_BEGIN namespace vrmv {
	#define VR_NAMESPACE_END }
#endif

VR_NAMESPACE_BEGIN

/**
 * @brief Global settings class
 */
class Settings {
public:

	/**
	 * @brief Singleton
	 */
	static Settings &getInstance () {
		static Settings instance;
		return instance;
	}

	virtual ~Settings () = default;

private:

	Settings ();

	/**
	 * Do not implement
	 */
	Settings(Settings const&) = delete;
	void operator=(Settings const&)  = delete;

public:

	/**
	 * Global settings as members
	 */
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	float FOV;
	float Z_NEAR;
	float Z_FAR;

	Eigen::Vector3f CAMERA_OFFSET;
	Eigen::Vector3f CAMERA_LOOK_AT;
	Eigen::Vector3f CAMERA_HEADS_UP;

	float MATERIAL_INTENSITY;
	Eigen::Vector3f LIGHT_INTENSITY;

	float MESH_DIAGONAL;
	bool SHOW_HANDS;

	float LEAP_ALPHA_SCALE;
	float LEAP_TO_WORLD_SCALE_3D;
	Leap::Vector LEAP_TO_WORLD_ORIGIN;
	bool LEAP_USE_PASSTHROUGH;
	float LEAP_CAMERA_SHIFT_X;
	float LEAP_CAMERA_SHIFT_Z;

	float GESTURES_PINCH_THRESHOLD;
};

VR_NAMESPACE_END
