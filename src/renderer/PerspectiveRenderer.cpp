#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height)
	, fH(tan(fov / 360 * M_PI) * zNear), fW(fH * aspectRatio), lightIntensity(Settings::getInstance().LIGHT_INTENSITY)
	, materialIntensity(Settings::getInstance().MATERIAL_INTENSITY), headsUp(Settings::getInstance().CAMERA_HEADS_UP)
	, lookAtPosition(Settings::getInstance().CAMERA_LOOK_AT)
	, cameraPosition(Settings::getInstance().CAMERA_OFFSET) {

	setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));
	setViewMatrix(lookAt(cameraPosition, lookAtPosition, headsUp));
}

void PerspectiveRenderer::preProcess () {
	Renderer::preProcess();

	// Upload mesh
	shader->bind();
	mesh->upload(shader);

	// Upload hands
	if (Settings::getInstance().SHOW_HANDS) {
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

	// Model material intensity
	shader->setUniform("intensity", materialIntensity);

	// Create virtual point light
	shader->setUniform("light.position", cameraPosition); // Camera position
	shader->setUniform("light.intensity", lightIntensity);
	shader->setUniform("light.intensity", lightIntensity);
}

void PerspectiveRenderer::draw() {
	shader->bind();
	
	// Draw the mesh
	shader->setUniform("modelMatrix", mesh->getModelMatrix());
	shader->setUniform("normalMatrix", mesh->getNormalMatrix());
	shader->setUniform("mvp", getMvp(mesh->getModelMatrix()));
	mesh->draw();

	if (Settings::getInstance().SHOW_HANDS) {
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
