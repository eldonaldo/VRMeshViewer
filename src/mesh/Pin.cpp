#include "mesh/Pin.hpp"

VR_NAMESPACE_BEGIN

Pin::Pin(Vector3f &pos) : Cube(Vector3f(-1.f, -1.f, -1.f), Vector3f(1.f, 1.f, 1.f)),  position(pos) {
	scale(0.012f, 0.012f, 0.012f);
}

Vector3f& Pin::getPosition() {
	return position;
}

VR_NAMESPACE_END
