#include "renderer/PerspectiveRenderer.hpp"

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: Renderer(shader), fov(fov), width(width), height(height), zNear(zNear), zFar(zFar), aspectRatio(width / height)
	, fH(tan(fov / 360 * M_PI) * zNear), fW(fH * aspectRatio), lightIntensity(Settings::getInstance().LIGHT_INTENSITY)
	, materialColor(Settings::getInstance().MATERIAL_COLOR), headsUp(Settings::getInstance().CAMERA_HEADS_UP)
	, lookAtPosition(Settings::getInstance().CAMERA_LOOK_AT)
	, cameraPosition(Settings::getInstance().CAMERA_OFFSET), GISphere(true) {

	setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));
	setViewMatrix(lookAt(cameraPosition, lookAtPosition, headsUp));
}

void PerspectiveRenderer::preProcess () {
	Renderer::preProcess();

	// Upload meshes
	shader->bind();
	mesh->upload(shader);
	sphere.upload(shader);
	sphere_large.upload(shader);
	sphere_small.upload(shader);
	
	// BBox
	BoundingBox3f mbbox = mesh->getBoundingBox();
	bbox = Cube(mbbox.min, mbbox.max);
	bbox.upload(shader);

	// Upload hands
	leftHand->upload(shader);
	rightHand->upload(shader);

	// Fake global illumination
	preProcessGI();

	// Anchor point on which tha model stands
	pedestal.update(mesh->getBoundingBox().min, mesh->getBoundingBox().max);
	pedestal.scale(0.7f, 20.f, 0.7f);
	pedestal.translate(0.f, 10.6f * (mesh->getBoundingBox().min.y() - mesh->getBoundingBox().max.y()) + 0.005f, 0.f);
	pedestal.upload(shader);

	// Material intensity
	shader->setUniform("materialColor", materialColor);

	// Create virtual point light
	shader->setUniform("light.intensity", lightIntensity);
	shader->setUniform("light.ambientCoefficient", Settings::getInstance().LIGHT_AMBIENT);
}

void PerspectiveRenderer::preProcessGI() {
	shader->bind();
	GISphere.scale(0.08f, 0.08f, 0.08f);
	GISphere.upload(shader);

	// Load environment HDR
	HDRLoaderResult result, resultDiffuse;
	HDRLoader::load(Settings::getInstance().GI_FILE.c_str(), result);
	HDRLoader::load(Settings::getInstance().GI_DIFFUSE_FILE.c_str(), resultDiffuse);
	environment = std::shared_ptr<float>(result.cols);
	environmentDiffuse = std::shared_ptr<float>(resultDiffuse.cols);

	// Texture uniforms
	envTexture = GLFramebuffer::createTexture(result.width, result.height);
	envDiffuseTexture = GLFramebuffer::createTexture(resultDiffuse.width, resultDiffuse.height);

	// Upload env
	glBindTexture(GL_TEXTURE_2D, envTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, result.width, result.height, 0, GL_RGB, GL_FLOAT, environment.get());

	// Upload env Diffuse 
	glBindTexture(GL_TEXTURE_2D, envDiffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resultDiffuse.width, resultDiffuse.height, 0, GL_RGB, GL_FLOAT, environmentDiffuse.get());

	// Pass to shader
	glBindVertexArray(GISphere.getVAO());
	GLuint env = glGetUniformLocation(shader->getId(), "env");
	GLuint envDiffuse = glGetUniformLocation(shader->getId(), "envDiffuse");
	glUniform1i(env, /*GL_TEXTURE*/0);
	glUniform1i(envDiffuse,/*GL_TEXTURE*/1);
	glBindVertexArray(0);
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
	if (Settings::getInstance().SHOW_SPHERE || Settings::getInstance().SPHERE_VISUAL_HINT) {
		sphereCenter = mesh->getBoundingBox().getCenter();
		sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_VISUAL_SCALE;

		sphere.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
		sphere.scale(Matrix4f::Identity(), sphereRadius, sphereRadius, sphereRadius);
		sphere.setRotationMatrix(r);
		
		// Debug spheres
		if (Settings::getInstance().SHOW_DEBUG_SPHERES) {
			// Large sphere
			sphereRadius_large = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_LARGE_SCALE;
			sphere_large.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
			sphere_large.scale(Matrix4f::Identity(), sphereRadius_large, sphereRadius_large, sphereRadius_large);
			sphere_large.setRotationMatrix(r);

			// Small sphere
			sphereRadius_small = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * Settings::getInstance().SPHERE_SMALL_SCALE;
			sphere_small.translate(sphereCenter.x(), sphereCenter.y(), sphereCenter.z());
			sphere_small.scale(Matrix4f::Identity(), sphereRadius_small, sphereRadius_small, sphereRadius_small);
			sphere_small.setRotationMatrix(r);
		}

		// Sphere blend
		if (Settings::getInstance().SHOW_SPHERE) {
			if (Settings::getInstance().SPHERE_ALPHA_BLEND_INTRO && Settings::getInstance().SPHERE_ALPHA_BLEND < 0.3f)
				Settings::getInstance().SPHERE_ALPHA_BLEND += 0.02f;
			else
				Settings::getInstance().SPHERE_ALPHA_BLEND_INTRO = false;
		}

		Settings::getInstance().SPHERE_DISPLAY = true;
	} else {
		Settings::getInstance().SPHERE_DISPLAY = false;
		if (!Settings::getInstance().SPHERE_VISUAL_HINT)
			Settings::getInstance().SPHERE_ALPHA_BLEND = 0.f;
	}

	// BBox blend
	if (Settings::getInstance().MESH_DRAW_BBOX) {
		if (Settings::getInstance().BBOX_ALPHA_BLEND < 0.8f)
			Settings::getInstance().BBOX_ALPHA_BLEND += 0.04f;

		Settings::getInstance().MESH_DISPLAY_BBOX = true;
	} else {
		if (Settings::getInstance().BBOX_ALPHA_BLEND > 0.f)
			Settings::getInstance().BBOX_ALPHA_BLEND -= 0.04f;
		else
			Settings::getInstance().MESH_DISPLAY_BBOX = false;
	}

	// Create virtual point light
	Vector3f cp = cameraPosition;
	if (Settings::getInstance().USE_RIFT)
		cp += Vector3f(0.4f, 0.2f, 0.5f);

	shader->setUniform("light.position", cp);
	shader->setUniform("cameraPosition", cameraPosition);

	// Default no wireframe and bbox overlay
	shader->setUniform("simpleColor", false);
}

void PerspectiveRenderer::draw() {
	shader->bind();

	// OpengGL Settings
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Shader settings
	Settings::getInstance().MATERIAL_COLOR = Vector3f(0.8f, 0.8f, 0.8f);
	shader->setUniform("materialColor", Settings::getInstance().MATERIAL_COLOR);
	shader->setUniform("alpha", 1.f);
	shader->setUniform("enableGI", Settings::getInstance().USE_RIFT && Settings::getInstance().GI_ENABLED);

	// Draw the mesh
	if (Settings::getInstance().MESH_DRAW)
		mesh->draw(getViewMatrix(), getProjectionMatrix());

	// Draw global illumination sphere
	if (Settings::getInstance().USE_RIFT && Settings::getInstance().GI_ENABLED) {
		glCullFace(GL_FRONT);
		shader->setUniform("textureOnly", true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, envTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, envDiffuseTexture);
		GISphere.draw(getViewMatrix(), getProjectionMatrix());
		shader->setUniform("textureOnly", false);
		glCullFace(GL_BACK);
	}

	// Draw annotations
	if (pinList != nullptr && !pinList->empty()) {
		shader->setUniform("enableGI", false);
		for (auto &p : *pinList)
			p->draw(getViewMatrix(), getProjectionMatrix());
		shader->setUniform("enableGI", true);
	}

	// Bounding box
	if (Settings::getInstance().MESH_DISPLAY_BBOX) {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("simpleColor", true);
		shader->setUniform("materialColor", Vector3f(0.8980f, 0.f, 0.16862f));
		shader->setUniform("alpha", clamp(Settings::getInstance().BBOX_ALPHA_BLEND));

		bbox.update(mesh->getBoundingBox().min, mesh->getBoundingBox().max);
		bbox.draw(getViewMatrix(), getProjectionMatrix());

		shader->setUniform("materialColor", Settings::getInstance().MATERIAL_COLOR);
		shader->setUniform("simpleColor", false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}

	// Rotation sphere
	if ((Settings::getInstance().USE_LEAP && Settings::getInstance().SPHERE_DISPLAY && Settings::getInstance().ENABLE_SPHERE) || Settings::getInstance().SPHERE_VISUAL_HINT) {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader->setUniform("simpleColor", true);
		shader->setUniform("materialColor", Vector3f(0.28627f, 0.26666f, 0.26274f));
		if (Settings::getInstance().SPHERE_VISUAL_HINT)
			shader->setUniform("materialColor", Settings::getInstance().SPHERE_VISUAL_HINT_COLOR);

		// (Visual) Rotation sphere
		shader->setUniform("alpha", clamp(Settings::getInstance().SPHERE_ALPHA_BLEND));
		sphere.draw(getViewMatrix(), getProjectionMatrix());
		
		// Debug spheres
		if (Settings::getInstance().SHOW_DEBUG_SPHERES) {
			// Large
			shader->setUniform("alpha", 0.3f);
			shader->setUniform("materialColor", Vector3f(0.4f, 0.4f, 0.4f));
			sphere_large.draw(getViewMatrix(), getProjectionMatrix());

			// Small
			shader->setUniform("alpha", 0.3f);
			shader->setUniform("materialColor", Vector3f(0.6f, 0.f, 0.f));
			sphere_small.draw(getViewMatrix(), getProjectionMatrix());
		}

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
		Settings::getInstance().MATERIAL_COLOR = Vector3f(1.f, 1.f, 1.f);
		shader->setUniform("materialColor", Settings::getInstance().MATERIAL_COLOR);
		float ambient = Settings::getInstance().LIGHT_AMBIENT;
		glDisable(GL_CULL_FACE);
		shader->setUniform("enableGI", false);
		if (Settings::getInstance().GI_ENABLED) {
			shader->setUniform("light.ambientCoefficient", 0.05f);
			shader->setUniform("specular", true);
			shader->setUniform("enableGI", true);
		}

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

		shader->setUniform("light.ambientCoefficient", ambient);
		shader->setUniform("specular", false);
		glEnable(GL_CULL_FACE);
	}

	// Draw the anchor point
 	if (Settings::getInstance().SHOW_SOCKEL && Settings::getInstance().USE_RIFT && Settings::getInstance().GI_ENABLED) {
		glDisable(GL_CULL_FACE);
		if ((pedestal.getBoundingBox().overlaps(mesh->getBoundingBox()) || 
			rightHand->containsBBox(pedestal.getBoundingBox()) || 
			leftHand->containsBBox(pedestal.getBoundingBox())) && Settings::getInstance().SOCKEL_ALPHA_BLEND > 0.f)
			
			Settings::getInstance().SOCKEL_ALPHA_BLEND -= 0.02f;
		else if (Settings::getInstance().SOCKEL_ALPHA_BLEND <= 1.f)
			Settings::getInstance().SOCKEL_ALPHA_BLEND += 0.02f;

		shader->setUniform("alpha", clamp(Settings::getInstance().SOCKEL_ALPHA_BLEND));
		pedestal.draw(getViewMatrix(), getProjectionMatrix());
		glEnable(GL_CULL_FACE);
	}
}

void PerspectiveRenderer::cleanUp () {
	
}

VR_NAMESPACE_END
