#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height)
	, fH(tan(fov / 360 * M_PI) * zNear), fW(fH * aspectRatio) {
	
	// 1.f = 1 Unit = 1 meter
	cameraPosition = Vector3f(
		0.f, // No shift on x-axis
		0.15f, // Head is 15cm above object's center
		0.35f // We look from 35cm away to the object
	);

	lookAtPosition = Vector3f(0.f, 0.f, 0.f); // The object is positioned at (0, 0, 0)
	headsUp = Vector3f(0.f, 1.f, 0.f);
	lightIntensity = Vector3f(1.f, 1.f, 1.f);
	materialIntensity =  0.8f;

	setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));
	setViewMatrix(lookAt(cameraPosition, lookAtPosition, headsUp));
}

void PerspectiveRenderer::preProcess () {
	Renderer::preProcess();

	// Model material intensity
	shader->bind();
	shader->setUniform("intensity", materialIntensity);

	// Create virtual point light
	shader->setUniform("light.position", cameraPosition); // Camera position
	shader->setUniform("light.intensity", lightIntensity);

	// Upload mesh
	mesh->upload(shader);

	// Upload hands
	if (showHands) {
		leftHand->upload(shader);
		leftHand->translate(-5.f, 0.f, 0.f);

		rightHand->upload(shader);
		rightHand->translate(+5.f, 0.f, 0.f);
	}
}

void PerspectiveRenderer::update (Matrix4f &s, Matrix4f &r, Matrix4f &t) {
	// Mesh model matrix
	mesh->setScaleMatrix(s);
	mesh->setRotationMatrix(r);
	mesh->setTranslateMatrix(t);
}

void PerspectiveRenderer::draw() {
	shader->bind();
	
	// Draw the mesh
	shader->setUniform("modelMatrix", mesh->getModelMatrix());
	shader->setUniform("normalMatrix", mesh->getNormalMatrix());
	shader->setUniform("mvp", getMvp(mesh->getModelMatrix()));
	mesh->draw();

	if (showHands) {
		// Left hand
		shader->setUniform("modelMatrix", leftHand->getModelMatrix());
		shader->setUniform("normalMatrix", leftHand->getNormalMatrix());
		shader->setUniform("mvp", getMvp(leftHand->getModelMatrix()));
		leftHand->draw();

		// right hand
		shader->setUniform("modelMatrix", rightHand->getModelMatrix());
		shader->setUniform("normalMatrix", rightHand->getNormalMatrix());
		shader->setUniform("mvp", getMvp(rightHand->getModelMatrix()));
		rightHand->draw();
	}
}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
