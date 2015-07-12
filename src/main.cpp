#include <thread>
#include "common.hpp"
#include "Viewer.hpp"
#include "network/UDPSocket.hpp"

using namespace VR_NS;

void initOVR (ovrHmd &hmd) {
	ovr_Initialize();
	hmd = ovrHmd_Create(0);

	// Create debug hmd
	if (!hmd)
		hmd = ovrHmd_CreateDebug(ovrHmdType::ovrHmd_DK2);

	if (!hmd)
		throw VRException("Could not start the Rift");

	// Configure which sensors we need to have
	if (!ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0))
		throw VRException("The Rift does not support all of the necessary sensors");
}

int main (int argc, char *argv[]) {

	// Settings
	Settings &config = Settings::getInstance();

	// LibOVR need to be initialized before GLFW
	ovrHmd hmd;
	initOVR(hmd);

	try {

		// Nanogui init
		nanogui::init();

		// OpenGL context
		Viewer *viewer = new Viewer(config.TITLE, config.WINDOW_WIDTH, config.WINDOW_HEIGHT, hmd);

		// Render loop
		nanogui::mainloop();

		delete viewer;

		// Nanogui shutdown
		nanogui::shutdown();

	} catch (std::exception &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		std::cin.get();
		return -1;
	}

	return 0;
}
