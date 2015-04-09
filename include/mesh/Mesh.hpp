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
	 * @param V Vertices matrix
	 * @param F Indices (faces) matrix
	 */
	Mesh (MatrixXf &V, MatrixXu &F) : vertices(V), indices(F) { };

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
	 * @brief Returns the number of vertices
	 *
	 * @return Number of vertices
	 */
	const unsigned int getNumFaces () const {
		return indices.cols();
	}

private:
	MatrixXf vertices; ///< Nx3 vertice matrix
	MatrixXu indices; ///< Mx3 index matrix
};

VR_NAMESPACE_END
