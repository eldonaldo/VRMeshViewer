#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

Mesh::Mesh()
	: glPositionName("position"), glNormalName("normal"), glTexName("tex")
	, transMat(Matrix4f::Identity()), scaleMat(Matrix4f::Identity()), rotateMat(Matrix4f::Identity()) {

	// Initialize standard values
	vbo[VERTEX_BUFFER] = 0;
	vbo[TEXCOORD_BUFFER] = 0;
	vbo[NORMAL_BUFFER] = 0;
	vbo[INDEX_BUFFER] = 0;
	vao = 0;
}

Mesh::~Mesh () {
	if (vbo[VERTEX_BUFFER])
		glDeleteBuffers(1, &vbo[VERTEX_BUFFER]);
	if (vbo[TEXCOORD_BUFFER])
		glDeleteBuffers(1, &vbo[TEXCOORD_BUFFER]);
	if (vbo[NORMAL_BUFFER])
		glDeleteBuffers(1, &vbo[NORMAL_BUFFER]);
	if (vbo[INDEX_BUFFER])
		glDeleteBuffers(1, &vbo[INDEX_BUFFER]);
	if (vao)
		glDeleteVertexArrays(1, &vao);
}

void Mesh::draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	Matrix4f mvp = projectionMatrix * viewMatrix * getModelMatrix();
	shader->bind();
	shader->setUniform("modelMatrix", getModelMatrix());
	shader->setUniform("normalMatrix", getNormalMatrix());
	shader->setUniform("mvp", mvp);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, getTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}


void Mesh::upload(std::shared_ptr<GLShader> &s) {
	shader = s;

	// VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Positions
	glGenBuffers(1, &vbo[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, 3 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_V.data(), GL_DYNAMIC_DRAW);
	GLuint pp = glGetAttribLocation(s->getId(), glPositionName.c_str());
	glVertexAttribPointer(pp, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pp);

	// UV
	if (m_UV.cols() > 0) {
		glGenBuffers(1, &vbo[TEXCOORD_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 2 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_UV.data(), GL_DYNAMIC_DRAW);
		GLuint uvp = glGetAttribLocation(s->getId(), glTexName.c_str());
		glVertexAttribPointer(uvp, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(uvp);
	}

	// Normals
	if (m_N.cols() > 0) {
		glGenBuffers(1, &vbo[NORMAL_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[NORMAL_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 3 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_N.data(), GL_DYNAMIC_DRAW);
		GLuint np = glGetAttribLocation(s->getId(), glNormalName.c_str());
		glVertexAttribPointer(np, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(np);
	}

	// Indices
	glGenBuffers(1, &vbo[INDEX_BUFFER]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * m_F.cols() * sizeof(GLuint), (const uint8_t *)m_F.data(), GL_DYNAMIC_DRAW);
	
	// Reset state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

VR_NAMESPACE_END
