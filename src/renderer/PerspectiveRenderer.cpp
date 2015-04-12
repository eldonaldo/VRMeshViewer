#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float aspectRatio, float zNear, float zFar)
	: Renderer(shader), fov(fov), aspectRatio(aspectRatio), zNear(zNear), zFar(zFar), fH(tan(fov / 360 * M_PI ) * zNear), fW(fH * aspectRatio) {

	projectionMatrix = frustum(-fW, fW, -fH, fH, zNear, zFar);

	viewMatrix = lookAt(
		Vector3f(4, 3, -3), // Camera is at (4,3,-3), in world space
		Vector3f(0, 0, 0), // And looks at the origin
		Vector3f(0, 1, 0) // Head is up
	);

	modelMatrix = Matrix4f::Identity();

	mvp = projectionMatrix * viewMatrix * modelMatrix;
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
	shader->setUniform("light.position", Vector3f(4.0, 3.0, -3.0)); // Camera position
	shader->setUniform("light.intensity", Vector3f(1.0, 1.0, 1.0));
}

void PerspectiveRenderer::update () {
	mvp = projectionMatrix * viewMatrix * modelMatrix;

	shader->bind();
	shader->setUniform("modelMatrix", modelMatrix);
	shader->setUniform("mvp", mvp);
}

void PerspectiveRenderer::draw() {
	shader->bind();
	shader->drawIndexed(GL_TRIANGLES, 0, mesh->getNumFaces());
}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
