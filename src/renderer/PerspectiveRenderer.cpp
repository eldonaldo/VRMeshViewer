#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height), fH(tan(fov / 360 * M_PI ) * zNear), fW(fH * aspectRatio) {

	setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));

	setViewMatrix(lookAt(
		Vector3f(0, 0, 5), // Camera is at (0,0,10), in world space
		Vector3f(0, 0, 0), // And looks at the origin
		Vector3f(0, 1, 0) // Head is up
	));

	setModelMatrix(Matrix4f::Identity());
}

void PerspectiveRenderer::preProcess () {
	Renderer::preProcess();
	shader->bind();
	shader->uploadIndices(mesh->getIndices());
	shader->uploadAttrib("position", mesh->getVertices());
	shader->uploadAttrib("normal", mesh->getNormals());

	// Model material intensity
	shader->setUniform("intensity", 0.8f);

	// Create virtual point light
	shader->setUniform("light.position", Vector3f(0, 0, 5.0)); // Camera position
	shader->setUniform("light.intensity", Vector3f(1.0, 1.0, 1.0));
}

void PerspectiveRenderer::update () {
	// Need to send every time because of the arcball rotation
	shader->bind();
	shader->setUniform("modelMatrix", getModelMatrix());
	shader->setUniform("normalMatrix", getNormalMatrix());
	shader->setUniform("mvp", getMvp());
}

void PerspectiveRenderer::draw() {
	shader->bind();
	shader->drawIndexed(GL_TRIANGLES, 0, mesh->getNumFaces());
}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
