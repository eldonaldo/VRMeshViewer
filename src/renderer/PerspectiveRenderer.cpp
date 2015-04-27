#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height)
	, fH(tan(fov / 360 * M_PI) * zNear), fW(fH * aspectRatio), cameraPosition(0.f, 0.f, 5.f), lightIntensity(1.f, 1.f, 1.f), materialIntensity(0.8f) {
	
	setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));
	setViewMatrix(lookAt(
		cameraPosition, // Camera position
		Vector3f(0, 0, 0), // Look at
		Vector3f(0, 1, 0) // Heads up
	));

	setModelMatrix(Matrix4f::Identity());
}

void PerspectiveRenderer::preProcess () {
	Renderer::preProcess();

	shader->bind();
	shader->uploadIndices(mesh->getIndices());
	shader->uploadAttrib("position", mesh->getVertexPositions());
	shader->uploadAttrib("normal", mesh->getVertexNormals());

	// Model material intensity
	shader->setUniform("intensity", materialIntensity);

	// Create virtual point light
	shader->setUniform("light.position", cameraPosition); // Camera position
	shader->setUniform("light.intensity", lightIntensity);
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
	shader->drawIndexed(GL_TRIANGLES, 0, mesh->getTriangleCount());
}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
