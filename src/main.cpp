#include "common.hpp"
#include "Viewer.hpp"
#include "mesh/WavefrontObj.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "renderer/RiftRenderer.hpp"
#include "leap/LeapListener.hpp"
#include "network/UDPSocket.hpp"
#include <thread>

using namespace VR_NS;

bool parseArgs (int argc, char *argv[]) {
	if (argc < 2)
		return false;

	Settings::getInstance().USE_RIFT = (std::string(argv[1]) == "3d" ? true : false);
	Settings::getInstance().MODEL = std::string(argv[2]);

	if (argc > 3)
		Settings::getInstance().ANNOTATIONS = std::string(argv[3]);

	// Networking args
	Settings::getInstance().NETWORK_ENABLED = false;
	if (argc >= 6) {
		Settings::getInstance().NETWORK_ENABLED = true;
		Settings::getInstance().NETWORK_MODE = NETWORK_MODES::CLIENT;
		Settings::getInstance().NETWORK_PORT = std::atoi(argv[5]);
		if (std::string(argv[4]) == "server") {
			Settings::getInstance().NETWORK_MODE = NETWORK_MODES::SERVER;
			Settings::getInstance().NETWORK_IP = std::string(argv[6]);
		}
	}

	return true;
}

int main (int argc, char *argv[]) {

	// Args
	if (!parseArgs(argc, argv)) {
		std::cout << "Usage: VRMeshViewer <3d|2d> <model.obj> [<none|annotations.txt>] [<client|server> <UDP-port> <ip-address>]" << std::endl;
		return -1;
	}

	// Settings
	int &width = Settings::getInstance().WINDOW_WIDTH, &height = Settings::getInstance().WINDOW_HEIGHT;
	float &fov = Settings::getInstance().FOV, &zNear = Settings::getInstance().Z_NEAR, &zFar = Settings::getInstance().Z_FAR;

	try {

		// This sets up the OpenGL context and needs the be first call
		Viewer viewer("Virtual Reality Mesh Viewer", width, height, false);

		// Create shader
		std::shared_ptr<GLShader> shader = std::make_shared<GLShader>();
		shader->initFromFiles("std-shader", "resources/shader/std-vertex-shader.glsl", "resources/shader/std-fragment-shader.glsl");

		// Create an appropriate renderer
		std::unique_ptr<Renderer> renderer;
		if (Settings::getInstance().USE_RIFT)
			renderer = std::unique_ptr<Renderer>(new RiftRenderer(shader, fov, width, height, zNear, zFar));
		else
			renderer = std::unique_ptr<Renderer>(new PerspectiveRenderer(shader, fov, width, height, zNear, zFar));

		// Load mesh
//		Settings::getInstance().MODEL = "resources/models/dragon/dragon.obj";
//		Settings::getInstance().MODEL = "resources/models/ironman/ironman.obj";
//		Settings::getInstance().MODEL = "resources/models/muro/muro.obj";
		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>(Settings::getInstance().MODEL);

		// Create Leap listener
		std::unique_ptr<LeapListener> leap(new LeapListener(Settings::getInstance().USE_RIFT));
		viewer.attachLeap(leap);

		// Networking
		std::unique_ptr<std::thread> netThread;
		asio::io_service io_service;
		asio::io_service::work work(io_service);

		/**
		 * If we're the server we need to choose another port.
		 * This is for debugging purposes only to run the application twice
		 * on the same machine and connect them together.
		 */
		short listenPort = Settings::getInstance().NETWORK_PORT;
		if (Settings::getInstance().NETWORK_MODE == NETWORK_MODES::SERVER)
			listenPort = Settings::getInstance().NETWORK_PORT - 1;
		UDPSocket socket(io_service, listenPort);

		// Setup networking
		if (Settings::getInstance().NETWORK_ENABLED) {

			// Print info
			std::cout << "Network[\n" <<
			"  Mode: " << (Settings::getInstance().NETWORK_MODE == 0 ? "Server" : "Client") << ",\n" <<
			"  Endpoint: " << Settings::getInstance().NETWORK_IP << ":" << Settings::getInstance().NETWORK_PORT << "\n" <<
			"]" << std::endl;

			// Run the network listener in a separate thread
			netThread = std::unique_ptr<std::thread>(new std::thread([&] {
				io_service.run();
			}));

			viewer.attachSocket(socket);
		}

		// Load annotations, if any, and run
		if (Settings::getInstance().ANNOTATIONS != "none")
			viewer.loadAnnotations(Settings::getInstance().ANNOTATIONS);

		viewer.display(mesh, renderer);

		// Stop networking and join to main thread
		if (Settings::getInstance().NETWORK_ENABLED) {
			io_service.stop();
			netThread->join();
		}

	} catch (std::exception &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		std::cin.get();
		return -1;
	}

	return 0;
}
