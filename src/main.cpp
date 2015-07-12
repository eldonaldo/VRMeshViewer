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
	std::string title = Settings::getInstance().TITLE;
	int width = Settings::getInstance().WINDOW_WIDTH;
	int height = Settings::getInstance().WINDOW_HEIGHT;

	// LibOVR need to be initialized before GLFW
	ovrHmd hmd;
	initOVR(hmd);

	try {

		// Nanogui init
		nanogui::init();

		// This sets up the OpenGL context
		Viewer *viewer = new Viewer(title, width, height, hmd);

		// Networking
//		std::unique_ptr<std::thread> netThread;
//		asio::io_service io_service;
//		asio::io_service::work work(io_service);

//		/**
//		 * If we're the server we need to choose another port.
//		 * This is for debugging purposes only to run the application twice
//		 * on the same machine and connect them together.
//		 */
//		short listenPort = Settings::getInstance().NETWORK_PORT;
//		if (Settings::getInstance().NETWORK_MODE == NETWORK_MODES::SERVER)
//			listenPort = Settings::getInstance().NETWORK_PORT - 1;
//		UDPSocket socket(io_service, listenPort);

		// Setup networking
		if (Settings::getInstance().NETWORK_ENABLED) {

//			// Print info
//			std::cout << "Network[\n" <<
//			"  Mode: " << (Settings::getInstance().NETWORK_MODE == 0 ? "Server" : "Client") << ",\n" <<
//			"  Endpoint: " << Settings::getInstance().NETWORK_IP << ":" << Settings::getInstance().NETWORK_PORT << "\n" <<
//			"]" << std::endl;

//			// Run the network listener in a separate thread
//			netThread = std::unique_ptr<std::thread>(new std::thread([&] {
//				io_service.run();
//			}));

//			viewer->attachSocket(socket);
		}

//		// Load annotations, if any, and run
//		if (Settings::getInstance().ANNOTATIONS != "none")
//			viewer->loadAnnotations(Settings::getInstance().ANNOTATIONS);

		nanogui::mainloop();

//		// Stop networking and join to main thread
//		if (Settings::getInstance().NETWORK_ENABLED) {
//			io_service.stop();
//			netThread->join();
//		}

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
