#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height)
	, fH(tan(fov / 360 * M_PI) * zNear), fW(fH * aspectRatio), lightIntensity(Settings::getInstance().LIGHT_INTENSITY)
	, materialColor(Settings::getInstance().MATERIAL_COLOR), headsUp(Settings::getInstance().CAMERA_HEADS_UP)
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
		rightHand->upload(shader);
	}
}

void PerspectiveRenderer::update (Matrix4f &s, Matrix4f &r, Matrix4f &t) {
	// Mesh model matrix
	mesh->setScaleMatrix(s);
	mesh->setRotationMatrix(r);
	mesh->setTranslateMatrix(t);

	// Material intensity
	shader->setUniform("materialColor", materialColor);

	// Create virtual point light
	shader->setUniform("light.position", cameraPosition);
	shader->setUniform("light.intensity", lightIntensity);

	// Default no wireframe and bbox overlay
	shader->setUniform("wireframe", false);
	shader->setUniform("bbox", false);
}

void PerspectiveRenderer::draw() {
	shader->bind();

	// Draw the mesh
	if (Settings::getInstance().MESH_DRAW) {
		shader->setUniform("alpha", 1.f);
		mesh->draw(getViewMatrix(), getProjectionMatrix());
	}

	/**
	 * Draw bounding box
	 *
	 * I know this is far from optimal but since the bbox is only
	 * for debugging purposes i upload the data every draw because
	 * I don't have access to the entire mesh state.
	 */
	if (Settings::getInstance().MESH_DRAW_BBOX) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("bbox", true);

		// Bounding box
		BoundingBox3f mbbox = mesh->getBoundingBox();
		bbox.releaseBuffers();
		bbox = Cube(mbbox.min, mbbox.max);
		bbox.upload(shader);

		bbox.draw(getViewMatrix(), getProjectionMatrix());
		shader->setUniform("bbox", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Draw wireframe overlay for debugging
	if (Settings::getInstance().MESH_DRAW_WIREFRAME) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("wireframe", true);
		mesh->draw(getViewMatrix(), getProjectionMatrix());
		shader->setUniform("wireframe", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Draw hands
	if (Settings::getInstance().SHOW_HANDS) {
		shader->setUniform("alpha", leftHand->confidence * Settings::getInstance().LEAP_ALPHA_SCALE);
		leftHand->draw(getLeapViewMatrix(), getProjectionMatrix());

		shader->setUniform("alpha", rightHand->confidence * Settings::getInstance().LEAP_ALPHA_SCALE);
		rightHand->draw(getLeapViewMatrix(), getProjectionMatrix());
	}
}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
