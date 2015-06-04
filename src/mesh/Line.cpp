#include "mesh/Line.hpp"

VR_NAMESPACE_BEGIN

Line::Line() : Line(Vector3f(0.f, 1.f, 0.f), Vector3f(0.f, -1.f, 0.f)) {

}

Line::Line(Vector3f a, Vector3f b) : Mesh() {
	m_V.resize(3, 2);
	m_V.col(0) = a;
	m_V.col(1) = b;
}

void Line::draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix) {
	Matrix4f mm = getModelMatrix();
	Matrix4f mvp = projectionMatrix * viewMatrix * mm;

	// Transform bounding box ... ok to do not on the GPU because its a transformation of only two points ...
	m_bbox.transformAxisAligned(mm);

	shader->bind();
	shader->setUniform("modelMatrix", mm);
	shader->setUniform("normalMatrix", getNormalMatrix());
	shader->setUniform("mvp", mvp);

	glLineWidth(20.f);
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

VR_NAMESPACE_END
