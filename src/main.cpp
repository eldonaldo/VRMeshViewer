#include "common.hpp"
#include "Viewer.hpp"
#include "GLUtil.hpp"

using namespace VR_NS;

int main (int argc, char *argv[]) {
	try {

//		// Create an appropriate renderer
//		float ratio = float(WINDOW_WIDTH / WINDOW_HEIGHT);
//		std::unique_ptr<Renderer> renderer(new PerspectiveRenderer(30.0, ratio, 1.0, 1000.0));
//
//		// Use (standard) phong shading
//		std::shared_ptr<GLSLProgram> shader = std::make_shared<GLSLProgram>(STD_VERTEX_SHADER, STD_FRAGMENT_SHADER);
//
//		// We receive the viewer instance
		Viewer viewer = Viewer("Virtual Reality Mesh Viewer", 800, 600);

		GLShader shader;
		shader.initFromFiles("Standard shader", "resources/shader/vertex-shader.glsl", "resources/shader/fragment-shader.glsl");
		Vector3f f(0.0, 0.0, 0.0);
//
//		// Configure
//		viewer.setRenderer(renderer);
//		viewer.setShader(shader);
//
//		// Run
//		viewer.display();

	} catch (std::runtime_error &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		return -1;
	}

	return 0;
}
