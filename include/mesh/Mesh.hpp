#pragma once

#include "common.hpp"
#include "mesh/BBox.hpp"
#include "mesh/kdtree.hpp"
#include "Eigen/Geometry"
#include "GLUtil.hpp"

VR_NAMESPACE_BEGIN

/**
 * \brief Triangle mesh
 *
 * This class stores a triangle mesh object and provides numerous functions
 * for querying the individual triangles. Subclasses of \c Mesh implement
 * the specifics of how to create its contents (e.g. by loading from an
 * external file)
 */

class Mesh {

public:
	
	/// Default constructor
	Mesh();

    /// Release all memory
    virtual ~Mesh();

    /// Release all memory
    void releaseBuffers ();

    /// Return the total number of triangles in this hsape
    uint32_t getTriangleCount() const { return (uint32_t) m_F.cols(); }

    /// Return the total number of vertices in this hsape
    uint32_t getVertexCount() const { return (uint32_t) m_V.cols(); }

    /// Return an axis-aligned bounding box of the entire mesh
    BoundingBox3f &getBoundingBox() { return m_bbox; }

    /// Return a pointer to the vertex positions
    const MatrixXf &getVertexPositions() const { return m_V; }

    /// Reset vertex positions
    void setVertexPositions (const MatrixXf &m) throw () {
    	if (m.cols() == m_V.cols() && m.rows() == m_V.rows())
    		m_V = m;
    	else
		    std::runtime_error("Can't set position matrix. Dimensions do not match.");
    }

    /// Return a pointer to the vertex normals (or \c nullptr if there are none)
    const MatrixXf &getVertexNormals() const { return m_N; }

    /// Return a pointer to the texture coordinates (or \c nullptr if there are none)
    const MatrixXf &getVertexTexCoords() const { return m_UV; }

    /// Return a pointer to the triangle vertex index list
    const MatrixXu &getIndices() const { return m_F; }

    /// Return the name of this mesh
    const std::string &getName() const { return m_name; }

    /// Return the kd-tree
    KDTree &getKDTree () { return kdtree; }

    /// Return a human-readable summary of this instance
	std::string toString() const {
		return
			"Mesh[\n"
			"  name = \""+m_name+"\",\n"
			"  vertexCount = "+std::to_string(m_V.cols())+",\n"
			"  triangleCount = "+std::to_string(m_F.cols())+"\n"
			"]";
	}

	/**
	 * @return Model Matrix
	 */
	virtual Matrix4f getModelMatrix();

	/**
	 * @return Transpose inverse model matrix
	 */
	virtual Matrix3f getNormalMatrix();

	/// Upload the mesh (positions, normals, indices and uv) to the shader
	virtual void upload(std::shared_ptr<GLShader> &s);

	/// Draw to the currently bounded shader
	virtual void draw(const Matrix4f &viewMatrix, const Matrix4f &projectionMatrix);

	/// Sets translation matrix
	virtual void setTranslateMatrix (Matrix4f &t);

	/// Sets scale matrix
	virtual void setScaleMatrix(Matrix4f &t);

	/// Sets rotation matrix
	virtual void setRotationMatrix(Matrix4f &t);

	/// Translate x, y, z
	virtual void translate(float x, float y, float z);

	/// Scale equally
	virtual void scale(float s);

	/// Scale x, y, z
	virtual void scale(float x, float y, float z);
	/// Scale x, y, z

	virtual void scale(Matrix4f mat, float x, float y, float z);

	/// Rotation around roll, pitch and yaw in radiants
	virtual void rotate(float roll, float pitch, float yaw);

	/// Rotation around roll, pitch and yaw in radiants
	virtual void rotate(float roll, Vector3f vr, float pitch, Vector3f vp, float yaw, Vector3f vy);

	/// Rotation round axis of angle radians
	virtual void rotate(float angle, Vector3f axis);

protected:
    std::string m_name;                  ///< Identifying name
    MatrixXf      m_V;                   ///< Vertex positions
    MatrixXf      m_N;                   ///< Vertex normals
    MatrixXf      m_UV;                  ///< Vertex texture coordinates
    MatrixXu      m_F;                   ///< Faces
    BoundingBox3f m_bbox;                ///< Bounding box of the mesh
	Matrix4f transMat;
	Matrix4f scaleMat;
	Matrix4f rotateMat;
	Matrix4f mmCache;
	bool mmChanged;

	enum BUFFERS {
		VERTEX_BUFFER,  //!< VERTEX_BUFFER
		TEXCOORD_BUFFER,//!< TEXCOORD_BUFFER
		NORMAL_BUFFER,  //!< NORMAL_BUFFER
		INDEX_BUFFER    //!< INDEX_BUFFER
	};
	GLuint vao;
	GLuint vbo[4];
	std::string glPositionName;
	std::string glNormalName;
	std::string glTexName;
	std::shared_ptr<GLShader> shader;
	KDTree kdtree;
};

VR_NAMESPACE_END
