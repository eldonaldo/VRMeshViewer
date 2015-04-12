#include "renderer/RiftRenderer.hpp"

VR_NAMESPACE_BEGIN

void RiftRenderer::preProcess () {
	PerspectiveRenderer::preProcess();
}

void RiftRenderer::update () {
	PerspectiveRenderer::update();
}

void RiftRenderer::draw() {
	PerspectiveRenderer::draw();
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
