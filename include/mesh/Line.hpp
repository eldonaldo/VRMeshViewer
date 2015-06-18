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
	
	void update (Vector3f &a, Vector3f &b);
	virtual void upload(std::shared_ptr<GLShader> &s);
	virtual void draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);

protected:
	bool buffersAllocated;
};

VR_NAMESPACE_END
