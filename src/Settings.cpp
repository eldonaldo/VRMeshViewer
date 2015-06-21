#include "Settings.hpp"

VR_NAMESPACE_BEGIN

// Set global settings here
Settings::Settings () :

	// GLOBAL
	MODEL						(""),
	ANNOTATIONS					("None"),

	// WINDOW
	WINDOW_WIDTH				(1200),
	WINDOW_HEIGHT				(900),
	FOV							(60.f),
	Z_NEAR						(0.001f),
	Z_FAR						(100.f),

	// CAMERA, 1.f = 1 Unit = 1 meter
	USE_RIFT					(false),
	CAMERA_OFFSET				(0.f, 0.20f, 0.40f),
	CAMERA_LOOK_AT				(0.f, 0.f, 0.f),
	CAMERA_HEADS_UP				(0.f, 1.f, 0.f),

	// MATERIAL
	MATERIAL_COLOR				(0.8f, 0.8f, 0.8f),

	// LIGHTNING
	LIGHT_INTENSITY				(1.f, 1.f, 1.f),

	// MESH, 1.f = 1 Unit = 1 meter
	MESH_DRAW					(true),
	MESH_DIAGONAL				(0.35f),
	MESH_DRAW_WIREFRAME			(false),
	MESH_DRAW_BBOX				(false),
	SHOW_HANDS					(true),
	SHOW_SPHERE					(false),
	ENABLE_SPHERE				(true),

	// LEAP, 1.f = 1 Unit = 1 millimeter
	// PASSTHROUGH
	LEAP_USE_PASSTHROUGH		(true),
	LEAP_NO_HMD_OFFSET			(0.f, 0.20f, 0.f),
	LEAP_ALPHA_SCALE			(0.7f),
	LEAP_CAMERA_SHIFT_X			(0.006f),
	LEAP_CAMERA_SHIFT_Z			(-0.08f),
	LEAP_USE_LISTENER			(false),

	// GESTURES
	GESTURES_PINCH_THRESHOLD	(1.f),
	GESTURES_GRAB_THRESHOLD		(1.f),
	GESTURES_RELATIVE_TRANSLATE	(true),

	// NETWORKING, 1 Unit = 1 millimeter
	NETWORK_ENABLED				(false),
	NETWORK_LISTEN				(true),
	NETWORK_MODE				(0),
	NETWORK_PORT				(8888),
	NETWORK_IP					("127.0.0.1"),
	NETWORK_SEND_RATE			(10)
{}

VR_NAMESPACE_END
