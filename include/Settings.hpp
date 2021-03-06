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
	std::string MODEL;
	std::string ANNOTATIONS;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	float FOV;
	float Z_NEAR;
	float Z_FAR;

	bool USE_RIFT;
	Eigen::Vector3f CAMERA_OFFSET;
	Eigen::Vector3f CAMERA_LOOK_AT;
	Eigen::Vector3f CAMERA_HEADS_UP;

	Eigen::Vector3f MATERIAL_COLOR;
	Eigen::Vector3f MATERIAL_COLOR_ROTATION;
	bool ROTATION_ACTIVE;
	Eigen::Vector3f LIGHT_INTENSITY;
	float LIGHT_AMBIENT;

	bool MESH_DRAW;
	float MESH_DIAGONAL;
	bool MESH_DRAW_WIREFRAME;
	bool MESH_DRAW_BBOX;
	bool MESH_DISPLAY_BBOX;
	float BBOX_ALPHA_BLEND;
	bool SHOW_HANDS;
	bool SHOW_SPHERE;
	bool ENABLE_SPHERE;
	bool SHOW_SOCKEL;
	float SOCKEL_ALPHA_BLEND;

	bool USE_LEAP;
	bool LEAP_USE_PASSTHROUGH;
	float LEAP_ALPHA_SCALE;
	float LEAP_CAMERA_SHIFT_X;
	float LEAP_CAMERA_SHIFT_Z;
	bool LEAP_USE_LISTENER;
	Eigen::Vector3f LEAP_NO_HMD_OFFSET;

	double GESTURES_SCALE_TIME;
	double GESTURES_ROTATION_TIME;
	float GESTURES_PINCH_THRESHOLD;
	float GESTURES_GRAB_THRESHOLD;
	bool GESTURES_RELATIVE_TRANSLATE;
	float ANNOTATION_SEACH_RADIUS;

	bool NETWORK_ENABLED;
	bool NETWORK_LISTEN;
	short NETWORK_PORT;
	int NETWORK_MODE;
	std::string NETWORK_IP;
	long NETWORK_SEND_RATE;
	int NETWORK_BUFFER_SIZE;
	bool NETWORK_NEW_DATA;

	bool SHOW_DEBUG_SPHERES;
	float SPHERE_VISUAL_SCALE;
	float SPHERE_SMALL_SCALE;
	float SPHERE_SPINCH_SCALE;
	float SPHERE_MEDIUM_SCALE;
	float SPHERE_LARGE_SCALE;
	bool SPHERE_DISPLAY;
	float SPHERE_ALPHA_BLEND;
	bool SPHERE_ALPHA_BLEND_INTRO;
	bool SPHERE_VISUAL_HINT;
	Eigen::Vector3f SPHERE_VISUAL_HINT_COLOR;

	bool GI_ENABLED;
	std::string GI_FILE;
	std::string GI_DIFFUSE_FILE;

	double FRAME_TIME;
};

VR_NAMESPACE_END
