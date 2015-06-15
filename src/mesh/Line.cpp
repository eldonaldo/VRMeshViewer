#include "mesh/Line.hpp"

VR_NAMESPACE_BEGIN

Line::Line() : Line(Vector3f(0.f, 1.f, 0.f), Vector3f(0.f, -1.f, 0.f)) {

}

Line::Line(Vector3f &a, Vector3f &b) : Mesh() {
	m_V.resize(3, 2);
	m_V.col(0) = a;
	m_V.col(1) = b;
	buffersAllocated = false;
}

void Line::update(Vector3f &a, Vector3f &b) {
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

	if (!buffersAllocated) {
		glBufferData(GL_ARRAY_BUFFER, 3 * m_V.cols() * sizeof(GLfloat), (const uint8_t *)m_V.data(), GL_STREAM_DRAW);
	} else {
		// Update data only
		void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (ptr) {
			memcpy(ptr, (const uint8_t *)m_V.data(), 3 * m_V.cols() * sizeof(GLfloat));
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}
	}

	if (!buffersAllocated) {
		GLuint pp = glGetAttribLocation(s->getId(), glPositionName.c_str());
		glVertexAttribPointer(pp, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(pp);
	}

	// Reset state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	buffersAllocated = true;
}

void Line::draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	Matrix4f mv = projectionMatrix * viewMatrix;
	shader->bind();
	shader->setUniform("mvp", mv);

	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

VR_NAMESPACE_END
