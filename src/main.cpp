#include "common.hpp"
#include "Viewer.hpp"
#include "mesh/WavefrontObj.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "renderer/RiftRenderer.hpp"
#include "leap/LeapListener.hpp"

using namespace VR_NS;

int main (int argc, char *argv[]) {

	// Window width and height
	int width = 1200, height = 900;

	try {

		bool useRift = true;

		// This sets up the OpenGL context and needs the be first call
		Viewer viewer("Virtual Reality Mesh Viewer", width, height, useRift, false);

		// Create shader
		std::shared_ptr<GLShader> shader = std::make_shared<GLShader>();
		shader->initFromFiles("std-shader", "resources/shader/vertex-shader.glsl", "resources/shader/fragment-shader.glsl");

		// Create an appropriate renderer
		std::unique_ptr<Renderer> renderer;
		if (useRift)
			renderer = std::unique_ptr<Renderer>(new RiftRenderer(shader, 60.f, width, height, 0.01f, 100.f));
		else
			renderer = std::unique_ptr<Renderer>(new PerspectiveRenderer(shader, 60.f, width, height, 0.01f, 100.f));

		// Load mesh
		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/dragon/dragon.obj");
//		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>("resources/models/dragon/dragon-no-normals.obj");
		
		// Create Leap listener
		std::unique_ptr<LeapListener> leap(new LeapListener(useRift));
		viewer.attachLeap(leap);

		// Run
		viewer.display(mesh, renderer);

	} catch (std::runtime_error &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		std::cin.get();
		return -1;
	}

	return 0;
}
