#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Line
 *
 */
class Line : public Mesh {
public:
	
	Line();
	Line(Vector3f a, Vector3f b);
	virtual ~Line() = default;
	
	virtual void draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);

};

VR_NAMESPACE_END
