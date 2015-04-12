#include "mesh/ObjectLoader.hpp"

VR_NAMESPACE_BEGIN

// Need to redefine here. Otherwise the Eclipse indexer has some problems ... don't know why
typedef Eigen::Matrix<float, 3, 1> Normal3f;

std::shared_ptr<Mesh> ObjectLoader::loadOBJ (std::string path) throw () {
	using namespace std;

	ifstream file(path, ios::in);
	if (!file.is_open())
		VRException("Unable to load OBJ file: %s", path);

	// Temporary data structures
	string line;
	vector<Vector3f> vertices;
	vector<Vector3ui> indices;
	vector<Vector3ui> normalIndices;
	vector<Vector3ui> texIndices;
	vector<Normal3f> normals;
	vector<Vector2f> texCoords;

	// Acceleration data structure to calculate per vertex normal
	std::map<unsigned int, std::vector<unsigned int>> nTable;

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

			// Need to determine which format the faces are stored ([a b c] OR [a/b c/d e/f] OR [a/b/c d/e/f g/h/i] OR [a//c d//f g//i])
			if (a.find("/") != string::npos) {
				// Format [a/b c/d e/f] OR [a/b/c d/e/f g/h/i]
				string indexA = a.substr(0, a.find("/")), indexB = b.substr(0, b.find("/")), indexC = c.substr(0, c.find("/"));
				istringstream (indexA) >> ia; istringstream (indexB) >> ib; istringstream (indexC) >> ic;

				/* We need to parse the normal und tex coordinate indices as well */
				unsigned int na, nb, nc;
				string endA = a.substr(a.find("/") + 1, a.length()), endB = b.substr(b.find("/") + 1, b.length()), endC = c.substr(c.find("/") + 1, c.length());
				if (endA.find("/") == string::npos) {
					// Format [a/b d/e g/h]
					istringstream(endA) >> na; istringstream(endB) >> nb; istringstream(endC) >> nc;
				} else {
					if (endA.find("/") == 0) {
						// Format [a//b d//e g//h]
						istringstream(endA.substr(1)) >> na; istringstream(endB.substr(1)) >> nb; istringstream(endC.substr(1)) >> nc;
					} else {
						// Format [a/b/c d/e/f g/h/i]
						// Normal indices
						string normalA = endA.substr(endA.find("/") + 1, endA.length());
						string normalB = endB.substr(endB.find("/") + 1, endB.length());
						string normalC = endC.substr(endC.find("/") + 1, endC.length());

						// Tex coordinate indices
						unsigned int ta, tb, tc;
						string texA = endA.substr(0, endA.find("/"));
						string texB = endA.substr(0, endB.find("/"));
						string texC = endA.substr(0, endC.find("/"));
						istringstream(normalA) >> na; istringstream(normalB) >> nb; istringstream(normalC) >> nc;
						istringstream(texA) >> ta; istringstream(texB) >> tb; istringstream(texC) >> tc;
						texIndices.push_back(Vector3ui(ta, tb, tc));
					}
				}
				normalIndices.push_back(Vector3ui(na, nb, nc));
			} else {
				// Format [a b c]
				istringstream (a) >> ia; istringstream (b) >> ib; istringstream (c) >> ic;
			}

			// We need the array/vector index which is one below
			ia--; ib--; ic--;
			indices.push_back(Vector3ui(ia, ib, ic));

			// Update look up table
			nTable[ia].push_back(indices.size() - 1);
			nTable[ib].push_back(indices.size() - 1);
			nTable[ic].push_back(indices.size() - 1);
		} else if (line.substr(0, 3) == "vn ") {
			// Vertex normal
			istringstream s(line.substr(3));
			float x, y, z;
			s >> x; s >> y; s >> z;
			normals.push_back(Vector3f(x, y, z));
		} else if (line.substr(0, 3) == "vt ") {
			// Texture coordinates
			istringstream s(line.substr(3));
			float u, v, w = 0.0;
			s >> u; s >> v; s >> w;
			texCoords.push_back(Vector2f(u, v));
		} else if (line.substr(0, 7) == "mtllib ") {
			// Material library
			// TODO: Finish
			ObjectLoader::loadMTL(line.substr(7));

		}
	}

	file.close();

	// Calculate per vertex normals if not present
	if (normals.size() == 0) {
		std::vector<Normal3f> faceNormals(indices.size());

		// First we compute the per face normals
		for (unsigned int i = 0; i < indices.size(); i++) {
			Vector3f A = vertices[indices[i].x()], B = vertices[indices[i].y()], C = vertices[indices[i].z()];
			faceNormals[i] = (((B - A).cross(C - A)).normalized());
		}

		// Compute per vertex normals
		for (unsigned int i = 0; i < vertices.size(); i++) {
			Normal3f n(0.0, 0.0, 0.0);

			// Search adjacent faces for that vertex
			for (unsigned int j : nTable[i])
				n += faceNormals[j];

			// We normalize later
			normals.push_back(n);
		}
	}


	/* Copy contents into Eigen matrices */
	// Vertices
	unsigned int n = vertices.size();
	MatrixXf V(3, n);
	for (int i = 0; i < n; i++)
		V.col(i) = vertices[i];

	// Normals
	n = normals.size();
	MatrixXf N(3, n);
	for (int i = 0; i < n; i++) {
		N.col(i) = normals[i].normalized();
	}

	// Inddices
	unsigned int m = indices.size();
	MatrixXu F(3, m);
	for (int i = 0; i < m; i++)
		F.col(i) = indices[i];

	return make_shared<Mesh>(path, V, F, N);
}

void ObjectLoader::loadMTL (std::string path) throw () {
	using namespace std;
	ifstream file(path, ios::in);
	if (!file.is_open())
		VRException("Unable to load material file: %s", path);

	// Process input
	string line;
	while (file.good()) {
		getline(file, line);
		if (line.substr(0, 7) == "newmtl ") {
			// New material
			istringstream s(line.substr(7));
			string name;
			s >> name;
		}
	}
}

VR_NAMESPACE_END
