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
	sphere.upload(shader);

	// Upload hands
	if (Settings::getInstance().SHOW_HANDS) {
		leftHand->upload(shader);
		rightHand->upload(shader);
	}

	// Material intensity
	shader->setUniform("materialColor", materialColor);

	// Create virtual point light
	shader->setUniform("light.intensity", lightIntensity);
}

void PerspectiveRenderer::update(Matrix4f &s, Matrix4f &r, Matrix4f &t) {
	// Mesh model matrix
	mesh->setScaleMatrix(s);
	mesh->setRotationMatrix(r);
	mesh->setTranslateMatrix(t);

	// Update pins
	if (pinList != nullptr && !pinList->empty()) {
		for (auto &p : *pinList) {
			p->setScaleMatrix(s);
			p->setRotationMatrix(r);
			p->setTranslateMatrix(t);
		}
	}

	// Bounding sphere
	if (Settings::getInstance().SHOW_SPHERE) {
		sphereCenter = mesh->getBoundingBox().getCenter();
		sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * 0.5f;

		sphere.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
		sphere.scale(Matrix4f::Identity(), sphereRadius, sphereRadius, sphereRadius);
		sphere.setRotationMatrix(r);
	}

	// Create virtual point light
	shader->setUniform("light.position", cameraPosition);

	// Default no wireframe and bbox overlay
	shader->setUniform("simpleColor", false);
}

void PerspectiveRenderer::draw() {
	shader->bind();

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Shader options
	shader->setUniform("materialColor", Settings::getInstance().MATERIAL_COLOR);
	shader->setUniform("alpha", 1.f);

	// Draw the mesh
	if (Settings::getInstance().MESH_DRAW)
		mesh->draw(getViewMatrix(), getProjectionMatrix());

	// Draw annotations
	if (pinList != nullptr && !pinList->empty())
		for (auto &p : *pinList)
			p->draw(getViewMatrix(), getProjectionMatrix());

	/**
	 * I know this is far from optimal but since the bbox is only
	 * for debugging purposes i upload the data every draw because
	 * I don't have access to the entire mesh state.
	 */
	// Bounding box
	if (Settings::getInstance().MESH_DRAW_BBOX) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("simpleColor", true);
		shader->setUniform("materialColor", Vector3f(1.f, 0.f, 0.f));
		shader->setUniform("alpha", 1.f);

		BoundingBox3f mbbox = mesh->getBoundingBox();
		bbox = Cube(mbbox.min, mbbox.max);
		bbox.upload(shader);
		bbox.draw(getViewMatrix(), getProjectionMatrix());

		shader->setUniform("materialColor", Settings::getInstance().MATERIAL_COLOR);
		shader->setUniform("simpleColor", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Bounding sphere
	if (Settings::getInstance().USE_LEAP && Settings::getInstance().SHOW_SPHERE && Settings::getInstance().ENABLE_SPHERE) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("simpleColor", true);
		shader->setUniform("materialColor", Vector3f(0.3f, 0.3f, 0.3f));
		shader->setUniform("alpha", 0.3f);

		sphere.draw(getViewMatrix(), getProjectionMatrix());

		shader->setUniform("simpleColor", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Draw wireframe overlay
	if (Settings::getInstance().MESH_DRAW_WIREFRAME) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("simpleColor", true);
		shader->setUniform("materialColor", Vector3f(0.2f, 0.2f, 0.2f));
		shader->setUniform("alpha", 1.f);

		mesh->draw(getViewMatrix(), getProjectionMatrix());

		shader->setUniform("simpleColor", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Draw hands
	if (Settings::getInstance().USE_LEAP && Settings::getInstance().SHOW_HANDS) {
		shader->setUniform("materialColor", Vector3f(0.8f, 0.8f, 0.8f));
		shader->setUniform("alpha", leftHand->confidence * Settings::getInstance().LEAP_ALPHA_SCALE);
		if (Settings::getInstance().USE_RIFT)
			leftHand->draw(getLeapViewMatrix(), getProjectionMatrix());
		else
			leftHand->draw(getViewMatrix(), getProjectionMatrix());

		shader->setUniform("alpha", rightHand->confidence * Settings::getInstance().LEAP_ALPHA_SCALE);
		if (Settings::getInstance().USE_RIFT)
			rightHand->draw(getLeapViewMatrix(), getProjectionMatrix());
		else
			rightHand->draw(getViewMatrix(), getProjectionMatrix());
	}

	// Unbind the shader for other renderings
	shader->unbind();
}

void PerspectiveRenderer::cleanUp () {
	
}

VR_NAMESPACE_END
