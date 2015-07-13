#include <thread>
#include "common.hpp"
#include "Viewer.hpp"
#include "network/UDPSocket.hpp"

using namespace VR_NS;

int main (int argc, char *argv[]) {
		
	// Settings
	Settings &config = Settings::getInstance();

	try {

		// Nanogui init
		nanogui::init();

		// OpenGL context
		Viewer *viewer = new Viewer(config.TITLE, config.WINDOW_WIDTH, config.WINDOW_HEIGHT);
		viewer->drawAll();
		viewer->setVisible(true);
		
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
