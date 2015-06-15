#include "mesh/Line.hpp"

VR_NAMESPACE_BEGIN

Line::Line() : Line(Vector3f(0.f, 1.f, 0.f), Vector3f(0.f, -1.f, 0.f)) {

}

Line::Line(Vector3f a, Vector3f b) : Mesh() {
	m_V.resize(3, 2);
	m_V.col(0) = a;
	m_V.col(1) = b;
}

void Line::upload(std::shared_ptr<GLShader> &s) {
	shader = s;

	// VAO
	if (!buffersAllocated)
		glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Positions
	if (!buffersAllocated)
		glGenBuffers(1, &vbo[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, 3 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_V.data(), GL_DYNAMIC_DRAW);
	if (!buffersAllocated) {
		GLuint pp = glGetAttribLocation(s->getId(), glPositionName.c_str());
		glVertexAttribPointer(pp, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pp);
	}

	// Indices
	if (!buffersAllocated)
		glGenBuffers(1, &vbo[INDEX_BUFFER]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * m_F.cols() * sizeof(GLuint), (const uint8_t *)m_F.data(), GL_DYNAMIC_DRAW);

	buffersAllocated = true;
}

void Line::draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	Matrix4f mm = getModelMatrix();
	Matrix4f mvp = projectionMatrix * viewMatrix * mm;

	shader->bind();
	shader->setUniform("modelMatrix", mm);
	shader->setUniform("normalMatrix", getNormalMatrix());
	shader->setUniform("mvp", mvp);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

VR_NAMESPACE_END
