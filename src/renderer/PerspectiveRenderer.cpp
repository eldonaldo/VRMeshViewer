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
	sphere1.upload(shader);
	sphere0.upload(shader);

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
		sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_VISUAL_SCALE;

		sphere.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
		sphere.scale(Matrix4f::Identity(), sphereRadius, sphereRadius, sphereRadius);
		sphere.setRotationMatrix(r);




		sphereRadius1 = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_LARGE_SCALE;

		sphere1.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
		sphere1.scale(Matrix4f::Identity(), sphereRadius1, sphereRadius1, sphereRadius1);
		sphere1.setRotationMatrix(r);

		sphereRadius0 = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_SMALL_SCALE;

		sphere0.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
		sphere0.scale(Matrix4f::Identity(), sphereRadius0, sphereRadius0, sphereRadius0);
		sphere0.setRotationMatrix(r);
	}

	// Create virtual point light
	shader->setUniform("light.position", cameraPosition);

	// Default no wireframe and bbox overlay
	shader->setUniform("simpleColor", false);
}

void PerspectiveRenderer::draw() {
	shader->bind();

	// OpengGL Settings
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

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
		glDisable(GL_CULL_FACE);
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
		glEnable(GL_CULL_FACE);
	}

	// Bounding sphere
	if (Settings::getInstance().USE_LEAP && Settings::getInstance().SHOW_SPHERE && Settings::getInstance().ENABLE_SPHERE) {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("simpleColor", true);
		shader->setUniform("materialColor", Vector3f(0.3f, 0.3f, 0.3f));
		
		shader->setUniform("alpha", 0.3f);

		sphere.draw(getViewMatrix(), getProjectionMatrix());
		



		//shader->setUniform("alpha", 0.3f);
		//shader->setUniform("materialColor", Vector3f(0.4f, 0.4f, 0.4f));
		//
		//sphere1.draw(getViewMatrix(), getProjectionMatrix());

		//shader->setUniform("alpha", 0.3f);
		//shader->setUniform("materialColor", Vector3f(0.6f, 0.f, 0.f));

		//sphere0.draw(getViewMatrix(), getProjectionMatrix());

		shader->setUniform("simpleColor", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
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
		glDisable(GL_CULL_FACE);

		if (leftHand->visible) {
			shader->setUniform("alpha", leftHand->confidence * Settings::getInstance().LEAP_ALPHA_SCALE);
			if (Settings::getInstance().USE_RIFT && Settings::getInstance().LEAP_USE_PASSTHROUGH)
				leftHand->draw(getLeapViewMatrix(), getProjectionMatrix());
			else
				leftHand->draw(getViewMatrix(), getProjectionMatrix());
		}

		if (rightHand->visible) {
			shader->setUniform("alpha", rightHand->confidence * Settings::getInstance().LEAP_ALPHA_SCALE);
			if (Settings::getInstance().USE_RIFT && Settings::getInstance().LEAP_USE_PASSTHROUGH)
				rightHand->draw(getLeapViewMatrix(), getProjectionMatrix());
			else
				rightHand->draw(getViewMatrix(), getProjectionMatrix());
		}

		glEnable(GL_CULL_FACE);
	}
}

void PerspectiveRenderer::cleanUp () {
	
}

VR_NAMESPACE_END
