#include "mesh/ObjectLoader.hpp"

VR_NAMESPACE_BEGIN

std::shared_ptr<Mesh> ObjectLoader::loadOBJ (std::string path) throw () {
	using namespace std;

	ifstream file(path, ios::in);
	if (!file.is_open())
		VRException("Unable to load OBJ file: %s", path);

	// Temporary data structures
	string line;
	vector<Vector3f> vertices;
	vector<Vector3ui> indices;

	// Process input
	while (file.good()) {
		getline(file, line);

		if (line.substr(0, 2) == "v ") {
			// Vertices
			istringstream s(line.substr(2));
			float x, y, z, w = 1.0;
			s >> x; s >> y; s >> z; s >> w;
			vertices.push_back(Vector3f(x, y, z));
		} else if (line.substr(0,2) == "f ") {
			// Faces / Indexes
			istringstream s(line.substr(2));
			string a, b, c;
			unsigned int ia, ib, ic;
			s >> a; s >> b; s >> c;

			// Need to determine which format the faces are stored ([a b c] OR [a/b c/d e/f] OR [a/b/c d/e/f g/h/i])
			if (a.find("/") != string::npos) {
				// Format [a/b c/d e/f] OR [a/b/c d/e/f g/h/i]
				string indexA = a.substr(0, a.find("/")), indexB = b.substr(0, b.find("/")), indexC = c.substr(0, c.find("/"));
				istringstream (indexA) >> ia; istringstream (indexB) >> ib; istringstream (indexC) >> ic;

				/* We only need the first value, but maybe later we need the normals and textures as well */
				// TODO: Parse uv and normal indices
				if (false) {
					// Format [a/b/c d/e/f g/h/i]
				}
			} else {
				// Format [a b c]
				istringstream (a) >> ia; istringstream (b) >> ib; istringstream (c) >> ic;
			}

			// We need the array/vector index which is one below
			ia--; ib--; ic--;
			indices.push_back(Vector3ui(ia, ib, ic));
		} else if (line.substr(0, 3) == "vn ") {
			// Vertex normal
			// TODO: Parse normals
			istringstream s(line.substr(3));
		} else if (line.substr(0, 3) == "vt ") {
			// Vertex texture
			// TODO: Parse textures
			istringstream s(line.substr(3));
		}
	}

	file.close();

	// TODO: Interpolate normals if not available

	/* Copy contents into an Eigen matrix */
	unsigned int n = vertices.size();
	MatrixXf V(3, n);
	for (int i = 0; i < n; i++)
		V.col(i) = vertices[i];

	unsigned int m = indices.size();
	MatrixXu F(3, m);
	for (int i = 0; i < m; i++)
		F.col(i) = indices[i];

	return make_shared<Mesh>(V, F);
}

VR_NAMESPACE_END
