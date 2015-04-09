#include "common.hpp"
#include "Viewer.hpp"
#include "GLUtil.hpp"
#include "mesh/ObjectLoader.hpp"
#include "renderer/PerspectiveRenderer.hpp"

using namespace VR_NS;

int main (int argc, char *argv[]) {
	int width = 800, height = 600;

	try {

		// Must be first call. This sets up the OpenGL contenxt
		Viewer viewer("Virtual Reality Mesh Viewer", width, height);

		// Create shader
		std::shared_ptr<GLShader> shader = std::make_shared<GLShader>();
		shader->initFromFiles("std-shader",
			"/Users/Nico/Development/VRMeshViewer/resources/shader/vertex-shader.glsl",
			"/Users/Nico/Development/VRMeshViewer/resources/shader/fragment-shader.glsl"
		);

		// Create an appropriate renderer
		std::unique_ptr<Renderer> renderer(new PerspectiveRenderer(shader, 45.0, width / height, 0.1, 100.0));

		// Load mesh
		std::shared_ptr<Mesh> mesh = ObjectLoader::loadOBJ("resources/models/plane/plane.obj");

//		MatrixXf positions(3, 8);
////		MatrixXf positions(3, 4);
//		positions.col(0) <<  1, -1, -1;
//		positions.col(1) <<  1, -1,  1;
//		positions.col(2) << -1, -1,  1;
//		positions.col(3) << -1, -1, -1;
//		positions.col(4) <<  1,  1, -1;
//		positions.col(5) <<  1,  1,  1;
//		positions.col(6) << -1,  1,  1;
//		positions.col(7) << -1,  1, -1;
//
////		positions.col(0) << -1.0, -1.0, 0.0;
////		positions.col(1) <<  1.0, -1.0, 0.0;
////		positions.col(2) <<  1.0,  1.0, 0.0;
////		positions.col(3) << -1.0,  1.0, 0.0;
//
//		MatrixXu indices(3, 12);
////		MatrixXu indices(3, 2);
//		indices.col(0) << 5, 1, 4;
//		indices.col(1) << 5, 4, 8;
//		indices.col(2) << 3, 7, 8;
//		indices.col(3) << 3, 8, 4;
//		indices.col(4) << 2, 6, 3;
//		indices.col(5) << 6, 7, 3;
//		indices.col(6) << 1, 5, 2;
//		indices.col(7) << 5, 6, 2;
//		indices.col(8) << 5, 8, 6;
//		indices.col(9) << 8, 7, 6;
//		indices.col(10) << 1, 2, 3;
//		indices.col(11) << 1, 3, 4;
//
////		indices.col(0) << 0, 1, 3;
////		indices.col(1) << 1, 3, 2;
//
//		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(positions, indices);

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
