#include "Settings.hpp"

VR_NAMESPACE_BEGIN

// Set global settings here
Settings::Settings () :

	// WINDOW
	WINDOW_WIDTH				(1200),
	WINDOW_HEIGHT				(900),
	FOV							(60.f),
	Z_NEAR						(0.01f),
	Z_FAR						(100.f),

	// CAMERA, 1.f = 1 Unit = 1 meter
	CAMERA_OFFSET				(0.f, 0.10f, 0.30f),
	CAMERA_LOOK_AT				(0.f, 0.f, 0.f),
	CAMERA_HEADS_UP				(0.f, 1.f, 0.f),

	// MATERIAL
	MATERIAL_INTENSITY			(0.8f),

	// LIGHTNING
	LIGHT_INTENSITY				(1.f, 1.f, 1.f),

	// MESH, 1.f = 1 Unit = 1 meter
	MESH_DIAGONAL				(0.35f),
	SHOW_HANDS					(true),

	// LEAP, 1.f = 1 Unit = 1 milimeter
	// PASSTHROUGH
	LEAP_USE_PASSTHROUGH		(true),
	LEAP_ALPHA_SCALE			(0.7f),
	LEAP_CAMERA_SHIFT_X			(0.012f),
	LEAP_CAMERA_SHIFT_Z			(-0.08f),

	// GESTURES
	GESTURES_PINCH_THRESHOLD	(1.f),
	GESTURES_GRAB_THRESHOLD		(0.9f),

	// No HMD
	LEAP_TO_WORLD_SCALE_3D		(0.05f),
	LEAP_TO_WORLD_ORIGIN		(0.f, -300.f, 0.f)
{}

VR_NAMESPACE_END
