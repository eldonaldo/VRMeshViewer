#include "common.hpp"
#include "Viewer.hpp"
#include "GLUtil.hpp"
#include "mesh/ObjectLoader.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "renderer/RiftRenderer.hpp"

using namespace VR_NS;

int main (int argc, char *argv[]) {

	// Window width and height
	int width = 1200, height = 900;

	try {

		// Must be first call. This sets up the OpenGL context
		Viewer viewer("Virtual Reality Mesh Viewer", width, height);

		// Create shader
		std::shared_ptr<GLShader> shader = std::make_shared<GLShader>();
		shader->initFromFiles("std-shader", "resources/shader/vertex-shader.glsl", "resources/shader/fragment-shader.glsl");

		// Create an appropriate renderer
//		std::unique_ptr<Renderer> renderer(new PerspectiveRenderer(shader, 45.0, width, height, 0.01, 10000.0));
		std::unique_ptr<Renderer> renderer(new RiftRenderer(shader, 45.0, width, height, 0.01, 10000.0));

		// Load mesh
		std::shared_ptr<Mesh> mesh = ObjectLoader::loadOBJ("resources/models/dragon/dragon.obj");
//		std::shared_ptr<Mesh> mesh = ObjectLoader::loadOBJ("resources/models/capsule/capsule.obj");

		// Configure
		viewer.setRenderer(renderer);

		// Run
		viewer.display(mesh);

	} catch (std::runtime_error &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		return -1;
	}

	return 0;
}
