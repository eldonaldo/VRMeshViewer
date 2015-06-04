#include "mesh/Cube.hpp"

VR_NAMESPACE_BEGIN

Cube::Cube() : Cube(Vector3f(-1.f, -1.f, -1.f), Vector3f(1.f, 1.f, 1.f)) {

}

Cube::Cube(Vector3f min, Vector3f max) {
	std::string v =
	"v " + std::to_string(max.x()) + " " + std::to_string(min.y()) + " " + std::to_string(min.z()) + "\n" +
	"v " + std::to_string(max.x()) + " " + std::to_string(min.y()) + " " + std::to_string(max.z()) + "\n" +
	"v " + std::to_string(min.x()) + " " + std::to_string(min.y()) + " " + std::to_string(max.z()) + "\n" +
	"v " + std::to_string(min.x()) + " " + std::to_string(min.y()) + " " + std::to_string(min.z()) + "\n" +
	"v " + std::to_string(max.x()) + " " + std::to_string(max.y()) + " " + std::to_string(min.z()) + "\n" +
	"v " + std::to_string(max.x()) + " " + std::to_string(max.y()) + " " + std::to_string(max.z()) + "\n" +
	"v " + std::to_string(min.x()) + " " + std::to_string(max.y()) + " " + std::to_string(max.z()) + "\n" +
	"v " + std::to_string(min.x()) + " " + std::to_string(max.y()) + " " + std::to_string(min.z()) + "\n" +

	"v 1.000000 -1.000000 -1.000000" + "\n" +
	"v 1.000000 -1.000000 1.000000" + "\n" +
	"v -1.000000 -1.000000 1.000000" + "\n" +
	"v -1.000000 -1.000000 -1.000000" + "\n" +
	"v 1.000000 1.000000 -1.000000" + "\n" +
	"v 0.999999 1.000000 1.000001" + "\n" +
	"v -1.000000 1.000000 1.000000" + "\n" +
	"v -1.000000 1.000000 -1.000000" + "\n" +

	"vn 0.000000 0.000000 -1.000000" + "\n" +
	"vn -1.000000 -0.000000 -0.000000" + "\n" +
	"vn -0.000000 -0.000000 1.000000" + "\n" +
	"vn -0.000001 0.000000 1.000000" + "\n" +
	"vn 1.000000 -0.000000 0.000000" + "\n" +
	"vn 1.000000 0.000000 0.000001" + "\n" +
	"vn 0.000000 1.000000 -0.000000" + "\n" +
	"vn -0.000000 -1.000000 0.000000" + "\n" +

	"f 5//1 1//1 4//1" + "\n" +
	"f 5//1 4//1 8//1" + "\n" +
	"f 3//2 7//2 8//2" + "\n" +
	"f 3//2 8//2 4//2" + "\n" +
	"f 2//3 6//3 3//3" + "\n" +
	"f 6//4 7//4 3//4" + "\n" +
	"f 1//5 5//5 2//5" + "\n" +
	"f 5//6 6//6 2//6" + "\n" +
	"f 5//7 8//7 6//7" + "\n" +
	"f 8//7 7//7 6//7" + "\n" +
	"f 1//8 2//8 3//8" + "\n" +
	"f 1//8 3//8 4//8" + "\n";

	cout << v << endl;
	std::istringstream is(v);
	loadFromString(is);
}

VR_NAMESPACE_END
