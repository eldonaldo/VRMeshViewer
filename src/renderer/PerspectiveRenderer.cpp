#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height)
	, fH(tan(fov / 360 * M_PI) * zNear), fW(fH * aspectRatio) {
	
	// 1.f = 1 Unit = 1 cm
	cameraPosition = Vector3f(
		0.f, // No shift on x-axis
		20.f, // Head is 20cm above object
		20.f // We look from 20cm away to the object
	);

	lookAtPosition = Vector3f(0.f, 0.f, 0.f); // The object is positioned at (0, 0, 0)
	headsUp = Vector3f(0.f, 1.f, 0.f);
	lightIntensity = Vector3f(1.f, 1.f, 1.f);
	materialIntensity =  0.8f;

	setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));
	setViewMatrix(lookAt(cameraPosition, lookAtPosition, headsUp));
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
	Renderer::draw();
	shader->bind();
	shader->drawIndexed(GL_TRIANGLES, 0, mesh->getTriangleCount());
}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
