#pragma once

#include "common.hpp"
#include "mesh/BBox.hpp"
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

    /// Return the total number of triangles in this hsape
    uint32_t getTriangleCount() const { return (uint32_t) m_F.cols(); }

    /// Return the total number of vertices in this hsape
    uint32_t getVertexCount() const { return (uint32_t) m_V.cols(); }

    /// Return an axis-aligned bounding box of the entire mesh
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    /// Return a pointer to the vertex positions
    const MatrixXf &getVertexPositions() const { return m_V; }

    /// Reset vertex positions
    void setVertexPositions (const MatrixXf &m) throw () {
    	if (m.cols() == m_V.cols() && m.rows() == m_V.rows())
    		m_V = m;
    	else
    		VRException("Can't set position matrix. Dimensions do not match.");
    }

    /// Return a pointer to the vertex normals (or \c nullptr if there are none)
    const MatrixXf &getVertexNormals() const { return m_N; }

    /// Return a pointer to the texture coordinates (or \c nullptr if there are none)
    const MatrixXf &getVertexTexCoords() const { return m_UV; }

    /// Return a pointer to the triangle vertex index list
    const MatrixXu &getIndices() const { return m_F; }

    /// Return the name of this mesh
    const std::string &getName() const { return m_name; }

    /// Return a human-readable summary of this instance
	std::string toString() const {
		return tfm::format(
			"Mesh[\n"
			"  name = \"%s\",\n"
			"  vertexCount = %i,\n"
			"  triangleCount = %i\n"
			"]",
			m_name,
			m_V.cols(),
			m_F.cols()
		);
	}

	/**
	 * @param modelMatrix Model Matrix
	 */
	void setModelMatrix (const Matrix4f &modelMatrix);

	/**
	 * @return Model Matrix
	 */
	const Matrix4f &getModelMatrix () const {
		return modelMatrix;
	}

	/**
	 * @return Transpose inverse model matrix
	 */
	const Matrix3f &getNormalMatrix () const {
		return normalMatrix;
	}

	void upload(std::shared_ptr<GLShader> &s);

	void draw();
   
protected:
    std::string m_name;                  ///< Identifying name
    MatrixXf      m_V;                   ///< Vertex positions
    MatrixXf      m_N;                   ///< Vertex normals
    MatrixXf      m_UV;                  ///< Vertex texture coordinates
    MatrixXu      m_F;                   ///< Faces
    BoundingBox3f m_bbox;                ///< Bounding box of the mesh
    Matrix4f modelMatrix; 				 ///< Model matrix
	Matrix3f normalMatrix; 				 ///< Transpose inverse model matrix
	
	static enum BUFFERS {
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
};

VR_NAMESPACE_END
