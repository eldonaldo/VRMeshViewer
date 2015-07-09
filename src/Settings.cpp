#include "Settings.hpp"

VR_NAMESPACE_BEGIN

// Set global settings here
Settings::Settings () :

	// GLOBAL
	MODEL						(""),
	ANNOTATIONS					("none"),

	// WINDOW
	WINDOW_WIDTH				(1200),
	WINDOW_HEIGHT				(900),
	FOV							(60.f),
	Z_NEAR						(0.001f),
	Z_FAR						(12.f),

	// CAMERA, 1.f = 1 Unit = 1 meter
	USE_RIFT					(false),
	CAMERA_OFFSET				(0.f, 0.20f, 0.35f),
	CAMERA_LOOK_AT				(0.f, 0.f, 0.f),
	CAMERA_HEADS_UP				(0.f, 1.f, 0.f),

	// MATERIAL
	MATERIAL_COLOR				(0.8f, 0.8f, 0.8f),
	MATERIAL_COLOR_ROTATION		(0.7843f, 0.72941f, 0.65098f),

	// LIGHTNING
	LIGHT_INTENSITY				(0.9f, 0.9f, 0.9f),
	LIGHT_AMBIENT				(0.005f),

	// MESH, 1.f = 1 Unit = 1 meter
	MESH_DRAW					(true),
	MESH_DIAGONAL				(0.35f),
	MESH_DRAW_WIREFRAME			(false),
	MESH_DISPLAY_BBOX			(false), 
	MESH_DRAW_BBOX				(false),
	SHOW_HANDS					(true),
	SHOW_SPHERE					(false),
	ENABLE_SPHERE				(true),
	BBOX_ALPHA_BLEND			(0.f),

	// LEAP, 1.f = 1 Unit = 1 millimeter
	// PASSTHROUGH
	USE_LEAP					(false),
	LEAP_USE_PASSTHROUGH		(false),
	LEAP_NO_HMD_OFFSET			(0.f, 0.20f, 0.f),
	LEAP_ALPHA_SCALE			(0.7f),
	LEAP_CAMERA_SHIFT_X			(0.006f),
	LEAP_CAMERA_SHIFT_Z			(-0.08f),
	LEAP_USE_LISTENER			(false),

	// GESTURES, 1.f = 1 Unit = 1 Second
	GESTURES_SCALE_TIME			(0.6f),
	GESTURES_ROTATION_TIME		(0.7f),
	GESTURES_PINCH_THRESHOLD	(0.95f),
	GESTURES_GRAB_THRESHOLD		(1.f),
	GESTURES_RELATIVE_TRANSLATE	(true),
	ANNOTATION_SEACH_RADIUS		(0.005f), // Meter

	// NETWORKING, 1 length unit = 1 millimeter, 1 size unit = 1 byte
	NETWORK_ENABLED				(false),
	NETWORK_LISTEN				(true),
	NETWORK_MODE				(0),
	NETWORK_PORT				(8888),
	NETWORK_IP					("127.0.0.1"),
	NETWORK_SEND_RATE			(50),
	NETWORK_BUFFER_SIZE			(8192),
	NETWORK_NEW_DATA			(false),

	// ROTATION SPHERE SCALES
	SHOW_DEBUG_SPHERES			(false),
	SPHERE_SMALL_SCALE			(0.3f),
	SPHERE_MEDIUM_SCALE			(0.6f),
	SPHERE_VISUAL_SCALE			(0.4f),
	SPHERE_LARGE_SCALE			(0.9f),
	SPHERE_ALPHA_BLEND			(0.f),
	SPHERE_DISPLAY				(false),
	SPHERE_ALPHA_BLEND_INTRO	(false),
	SPHERE_VISUAL_HINT			(false),
	SPHERE_VISUAL_HINT_COLOR	(0.6f, 0.f, 0.f),

	// GLOBAL ILLUMINATION
	GI_ENABLED					(true),
	GI_FILE						("resources/hdr/loft.hdr"),
	GI_DIFFUSE_FILE				("resources/hdr/loft_diffuse.hdr")
{}

VR_NAMESPACE_END
