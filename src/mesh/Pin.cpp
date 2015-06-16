#include "mesh/Pin.hpp"

VR_NAMESPACE_BEGIN

Pin::Pin(Vector3f &pos) : Cube(Vector3f(-1.f, -1.f, -1.f), Vector3f(1.f, 1.f, 1.f)),  position(pos) {

}

Vector3f& Pin::getPosition() {
	return position;
}

Matrix4f Pin::getModelMatrix() {
	return transMat * rotateMat * scaleMat * VR_NS::translate(Matrix4f::Identity(), position) * VR_NS::scale(Matrix4f::Identity(), 0.012f, 0.012f, 0.012f);
}


VR_NAMESPACE_END
