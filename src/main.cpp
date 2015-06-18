#include "common.hpp"
#include "Viewer.hpp"
#include "mesh/WavefrontObj.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "renderer/RiftRenderer.hpp"
#include "leap/LeapListener.hpp"

using namespace VR_NS;

int main (int argc, char *argv[]) {

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
		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/dragon/dragon.obj");
//		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/ironman/ironman.obj");
//		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/muro/muro.obj");
//		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/ajax.obj");
//		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/kingkong.obj");

		// Create Leap listener
		std::unique_ptr<LeapListener> leap(new LeapListener(Settings::getInstance().USE_RIFT));
		viewer.attachLeap(leap);

		// Run
		//viewer.loadAnnotations("resources/models/dragon/dragon-annotations-7.txt");
		viewer.display(mesh, renderer);
	} catch (std::runtime_error &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		std::cin.get();
		return -1;
	}

	return 0;
}
