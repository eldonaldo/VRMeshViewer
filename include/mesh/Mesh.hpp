#pragma once

#include "common.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief A mesh is basically just a pair of two matrices (Vertices and Faces).
 *
 * This class stores two matrices, V and F. V is a Nx3 matrix and stores the vertices
 * row wise. V Mx3 is a matrix containing the 3 indices to V, which make up a face.
 */
class Mesh {
public:
	/**
	 * @brief Copy constructor
	 *
	 * @param name Mesh name/path
	 * @param V Vertices matrix
	 * @param F Indices (faces) matrix
	 */
	Mesh (std::string &name, MatrixXf &V, MatrixXu &F)
		: name(name), vertices(V), indices(F) {

	};

	/**
	 * @brief Copy constructor
	 *
	 * @param name Mesh name/path
	 * @param V Vertices matrix
	 * @param F Indices (faces) matrix
	 * @param N Normal matrix
	 */
	Mesh (std::string &name, MatrixXf &V, MatrixXu &F, MatrixXf &N)
		: name(name), vertices(V), indices(F), normals(N) {

	};

	/**
	 * @brief Default destructor
	 */
	virtual ~Mesh () = default;

	/**
	 * @brief Returns the indices matrix
	 *
	 * @return Indices matrix
	 */
	const MatrixXu& getIndices () const {
		return indices;
	}

	/**
	 * @brief Returns the vertices matrix
	 *
	 * @return Vertices matrix
	 */
	const MatrixXf& getVertices () const {
		return vertices;
	}

	/**
	 * @brief Returns the normal matrix
	 *
	 * @return Normal matrix
	 */
	const MatrixXf& getNormals () const {
		return normals;
	}

	/**
	 * @brief Returns the number of vertices
	 *
	 * @return Number of vertices
	 */
	unsigned int getNumFaces () const {
		return indices.cols();
	}

	/**
	 * @brief Returns some info about the mesh
	 *
	 * @return Brief mesh description
	 */
	std::string info () const {
		return tfm::format(
			"Mesh[\n"
			"  name = %s,\n"
			"  vertices = %d,\n"
			"  normals = %d,\n"
			"  faces = %d\n"
			"]",
			name,
			vertices.cols(),
			normals.cols(),
			indices.cols()
		);
	}

private:

	std::string name; ///< Object name
	MatrixXf vertices; ///< Nx3 vertex matrix
	MatrixXf normals; ///< Nx3 normal matrix
	MatrixXu indices; ///< Mx3 index matrix
};

VR_NAMESPACE_END
