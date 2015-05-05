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
	 * @return Model Matrix
	 */
	Matrix4f &getModelMatrix () {
		modelMatrix = scaleMat * rotateMat * transMat;
		return modelMatrix;
	}

	/**
	 * @return Transpose inverse model matrix
	 */
	Matrix3f &getNormalMatrix () {
		// Calculate normal matrix for normal transformation
		Matrix3f tmp = modelMatrix.topLeftCorner<3, 3>();
		Matrix3f inv = tmp.inverse();
		normalMatrix = inv.transpose();
		return normalMatrix;
	}

	/// Upload the mesh (positions, normals, indices and uv) to the shader
	void upload(std::shared_ptr<GLShader> &s);

	/// Draw to the currently bounded shader
	void draw();


	/// Sets translation matrix
	void setTranslateMatrix (Matrix4f t) { transMat = t; }

	/// Sets scale matrix
	void setScaleMatrix (Matrix4f t) { scaleMat = t; }

	/// Sets rotation matrix
	void setRotationMatrix (Matrix4f t) { rotateMat = t; }

	/// Translate x, y, z
	void translate (float x, float y, float z) {
		transMat = VR_NS::translate(Matrix4f::Identity(), Vector3f(x, y, z));
	}

	/// Scale equally
	void scale (float s){
		scaleMat = VR_NS::scale(Matrix4f::Zero(), s);
	}

	/// Scale x, y, z
	void scale (float x, float y, float z){
		scaleMat = VR_NS::scale(Matrix4f::Zero(), x, y, z);
	}

	/// Rotation around roll, pitch and yaw in degrees
	void rotate (float roll, float pitch, float yaw) {
		Eigen::AngleAxisf rollAngle(roll, Vector3f::UnitZ());
		Eigen::AngleAxisf yawAngle(yaw, Vector3f::UnitY());
		Eigen::AngleAxisf pitchAngle(pitch, Vector3f::UnitX());

		Quaternionf q = rollAngle * yawAngle * pitchAngle;
		rotateMat = Matrix4f::Identity();
		rotateMat.block<3, 3>(0, 0) = q.matrix();
	}
   
protected:
    std::string m_name;                  ///< Identifying name
    MatrixXf      m_V;                   ///< Vertex positions
    MatrixXf      m_N;                   ///< Vertex normals
    MatrixXf      m_UV;                  ///< Vertex texture coordinates
    MatrixXu      m_F;                   ///< Faces
    BoundingBox3f m_bbox;                ///< Bounding box of the mesh
    Matrix4f modelMatrix; 				 ///< Model matrix
	Matrix3f normalMatrix; 				 ///< Transpose inverse model matrix
	Matrix4f transMat;
	Matrix4f scaleMat;
	Matrix4f rotateMat;
	
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
};

VR_NAMESPACE_END
