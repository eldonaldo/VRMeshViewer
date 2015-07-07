#include "mesh/Cube.hpp"

VR_NAMESPACE_BEGIN

Cube::Cube() : Cube(Vector3f(-1.f, -1.f, -1.f), Vector3f(1.f, 1.f, 1.f)) {

}

void Cube::update(const Vector3f &min, const Vector3f &max) {
	std::string v =
	std::to_string(max.x()) + " " + std::to_string(min.y()) + " " + std::to_string(min.z()) + "\n" +
	std::to_string(max.x()) + " " + std::to_string(min.y()) + " " + std::to_string(max.z()) + "\n" +
	std::to_string(min.x()) + " " + std::to_string(min.y()) + " " + std::to_string(max.z()) + "\n" +
	std::to_string(min.x()) + " " + std::to_string(min.y()) + " " + std::to_string(min.z()) + "\n" +
	std::to_string(max.x()) + " " + std::to_string(max.y()) + " " + std::to_string(min.z()) + "\n" +
	std::to_string(max.x()) + " " + std::to_string(max.y()) + " " + std::to_string(max.z()) + "\n" +
	std::to_string(min.x()) + " " + std::to_string(max.y()) + " " + std::to_string(max.z()) + "\n" +
	std::to_string(min.x()) + " " + std::to_string(max.y()) + " " + std::to_string(min.z()) + "\n";

	std::istringstream is(v);
	std::string line_str;
	std::vector<Vector3f> newPos;
	while (std::getline(is, line_str)) {
		std::istringstream line(line_str);
		
		Point3f p;
		line >> p.x() >> p.y() >> p.z();
		newPos.push_back(p);
	}

	for (uint32_t i = 0; i < m_V.cols(); ++i)
		m_V.col(i) = newPos.at(vertices[i].p - 1);

	// Update data on GPU
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTEX_BUFFER]);
	// Update data only
	void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (ptr) {
		memcpy(ptr, (const uint8_t *)m_V.data(), 3 * m_V.cols() * sizeof(GLfloat));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Cube::upload(std::shared_ptr<GLShader> &s) {
	shader = s;
	shader->bind();

	// VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Positions
	glGenBuffers(1, &vbo[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, 3 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_V.data(), GL_STREAM_DRAW);
	GLuint pp = glGetAttribLocation(s->getId(), glPositionName.c_str());
	glVertexAttribPointer(pp, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pp);

	// UV
	if (m_UV.cols() > 0) {
		glGenBuffers(1, &vbo[TEXCOORD_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 2 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_UV.data(), GL_STATIC_DRAW);
		GLuint uvp = glGetAttribLocation(s->getId(), glTexName.c_str());
		glVertexAttribPointer(uvp, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(uvp);
	}

	// Normals
	if (m_N.cols() > 0) {
		glGenBuffers(1, &vbo[NORMAL_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[NORMAL_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 3 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_N.data(), GL_STATIC_DRAW);
		GLuint np = glGetAttribLocation(s->getId(), glNormalName.c_str());
		glVertexAttribPointer(np, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(np);
	}

	// Indices
	glGenBuffers(1, &vbo[INDEX_BUFFER]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * m_F.cols() * sizeof(GLuint), (const uint8_t *)m_F.data(), GL_STATIC_DRAW);

	// Reset state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

	std::istringstream is(v);
	loadFromString(is);
}

VR_NAMESPACE_END
