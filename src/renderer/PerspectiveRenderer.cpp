#include <renderer/PerspectiveRenderer.hpp>

VR_NAMESPACE_BEGIN

PerspectiveRenderer::PerspectiveRenderer (float fov, float aspectRatio, float zNear, float zFar)
	: Renderer(), fov(fov), aspectRatio(aspectRatio), zNear(zNear), zFar(zFar) {

//	// Calculate projection matrix
//	projectionMatrix = glm::perspective(this->fov, this->aspectRatio, this->zNear, this->zFar);
//
//	// Calculate view matrix
//	viewMatrix = glm::lookAt(
//		Vector3f(4, 3, -3), // Camera is at (4,3,-3), in world space
//		Vector3f(0, 0, 0), // And looks at the origin
//		Vector3f(0, 1, 0) // Head is up
//	);
//
//	// Model matrix : an identity matrix (model will be at the origin)
//	modelMatrix = Matrix4f(0.0f);
//	Renderer::update();
//
//	shader->updateUniform("mvp", mvp, EMatrix4);
}

void PerspectiveRenderer::update () {
}

void PerspectiveRenderer::preProcess () {

}

void PerspectiveRenderer::draw() {

}

void PerspectiveRenderer::cleanUp () {

}

VR_NAMESPACE_END
