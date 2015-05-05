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
	MESH_DIAGONAL				(0.35f)

{}

VR_NAMESPACE_END
