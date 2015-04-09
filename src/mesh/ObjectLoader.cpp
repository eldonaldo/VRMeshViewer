#include "mesh/ObjectLoader.hpp"

VR_NAMESPACE_BEGIN

std::shared_ptr<Mesh> ObjectLoader::loadOBJ (std::string path) throw () {
	std::ifstream file(path, std::ios::in);
	if (!file)
		VRException("Unable to load OBJ file: %s", path);

	std::string line;

	std::vector<Vector3f> vertices;
	std::vector<Vector3ui> indices;
	while (std::getline(file, line)) {
		if (line.substr(0, 2) == "v ") {
			// Vertices
			std::istringstream s(line.substr(2));
			float x, y, z;
			s >> x; s >> y; s >> z;
			vertices.push_back(Vector3f(x, y, z));
		} else if (line.substr(0,2) == "f ") {
			// Faces / Indexes
			std::istringstream s(line.substr(2));
			unsigned int a,b,c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			indices.push_back(Vector3ui(a, b, c));
		}
	}

	unsigned int n = vertices.size();
	MatrixXf V(n, 3);
	for (int i = 0; i < n; i++)
		V.row(i) = vertices[i];

	unsigned int m = indices.size();
	MatrixXu F(m, 3);
	for (int i = 0; i < m; i++)
		F.row(i) = indices[i];

	// TODO: Interpolate normals and read materials
	file.close();

//	std::cout << "Vertices: " << V << std::endl;
//	std::cout << "Faces: " << F << std::endl;
	return std::make_shared<Mesh>(V, F);
}

VR_NAMESPACE_END
