#pragma once

#include "common.hpp"
#include "GLUtil.hpp"
#include "mesh/Mesh.hpp"

VR_NAMESPACE_BEGIN

/**
 * Forward declaration
 */
class Viewer;

/**
 * @brief Available renderer types
 */
enum RendererType {
	ENormalRenderer,
	EHMDRenderer
};

/**
 *
 */
class Renderer {
public:
	/**
	 * @brief Default constructor
	 */
	Renderer (std::shared_ptr<GLShader> &s)
		: shader(s), FBWidth(0), FBHeight(0), window(nullptr), viewMatrix(Matrix4f::Identity())
		, modelMatrix(Matrix4f::Identity()), normalMatrix(Matrix3f::Identity()), projectionMatrix(Matrix4f::Identity()), mvp(Matrix4f::Identity())
		, hmd(nullptr) {

//		bboxShader = std::unique_ptr<GLShader>(new GLShader());
//		bboxShader->initFromFiles("bbox-shader", "resources/shader/bbox-vertex.glsl", "resources/shader/bbox-fragment.glsl");
	};

	/**
	 * @brief Default destructor
	 */
	virtual ~Renderer () {
		shader->free();
	}

	/**
	 * @brief Clears the buffers and background
	 *
	 * @param background Background color
	 */
	virtual void clear (Vector3f background) {
		glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	/**
	 * @brief Updates the state
	 *
	 * This method must be implemented by all subclasses. This method is
	 * always called before Renderer::draw();
	 */
	virtual void update () = 0;

	/**
	 * @brief Allows the renderer to do some processing
	 * 		  before the render loop is entered.
	 *
	 * The default implementation uploads the bounding box of the object.
	 */
	virtual void preProcess () {
//		if (mesh != nullptr) {
//			Point3f min = mesh->getBoundingBox().min;
//			Point3f max = mesh->getBoundingBox().max;
//
//			Vector4f vmin(min.x(), min.y(), min.z(), 1.f);
//			Vector4f vmax(max.x(), max.y(), max.z(), 1.f);
//			vmin = getModelMatrix() * vmin;
//			vmax = getModelMatrix() * vmax;
//
//			MatrixXf V(3, 8);
//			V.col(0) = Vector3f(min.x(), min.y(), min.z());
//			V.col(1) = Vector3f(min.x(), min.y(), max.z());
//			V.col(2) = Vector3f(max.x(), min.y(), max.z());
//			V.col(3) = Vector3f(max.x(), min.y(), min.z());
//			V.col(4) = Vector3f(max.x(), max.y(), max.z());
//			V.col(5) = Vector3f(max.x(), max.y(), min.z());
//			V.col(6) = Vector3f(min.x(), max.y(), min.z());
//			V.col(7) = Vector3f(min.x(), max.y(), max.z());
//
//			MatrixXu indices(2, 12);
//			indices.col(0) = Vector2ui(0, 1);
//			indices.col(1) = Vector2ui(1, 2);
//			indices.col(2) = Vector2ui(2, 3);
//			indices.col(3) = Vector2ui(3, 0);
//			indices.col(4) = Vector2ui(0, 6);
//			indices.col(5) = Vector2ui(1, 7);
//			indices.col(6) = Vector2ui(2, 4);
//			indices.col(7) = Vector2ui(3, 5);
//			indices.col(8) = Vector2ui(5, 6);
//			indices.col(9) = Vector2ui(6, 7);
//			indices.col(10) = Vector2ui(7, 4);
//			indices.col(11) = Vector2ui(4, 5);
//
//			bboxShader->bind();
//			bboxShader->uploadIndices(indices);
//			bboxShader->uploadAttrib("position", V);
//			bboxShader->setUniform("mvp", getMvp());
//
//			std::cout << V << "\n" << std::endl;
//			std::cout << indices << "\n" << std::endl;
//		}
	}

	/**
	 * @brief Draws the loaded data
	 *
	 * This method must be implemented by all subclasses. This method is
	 * called after Renderer::update();
	 */
	virtual void draw () {
//		if (mesh != nullptr) {
//			bboxShader->bind();
//			bboxShader->drawIndexed(GL_LINES, 0, 12);
//		}
	}

	/**
	 * @brief Sets the mesh
	 *
	 * @param m Mesh
	 */
	void setMesh (std::shared_ptr<Mesh> &m) {
		mesh = m;
	}

	/**
	 * @brief To the necessary clean up
	 */
	virtual void cleanUp () = 0;

	/**
	 * @return Brief info about the renderer
	 */
	virtual const std::string info () const = 0;

	/**
	 * @brief Returns the class type
	 */
	virtual const RendererType getClassType () const {
		return ENormalRenderer;
	}

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

	/**
	 * @param modelMatrix Model Matrix
	 */
	void setModelMatrix (const Matrix4f &modelMatrix) {
		this->modelMatrix = modelMatrix;
		mvp = projectionMatrix * viewMatrix * modelMatrix;

		// Calculate normal matrix for normal transformation
		Matrix3f tmp = modelMatrix.topLeftCorner<3, 3>();
		Matrix3f inv = tmp.inverse();
		normalMatrix = inv.transpose();
	}

	/**
	 * @return MVP Matrix
	 */
	const Matrix4f &getMvp () const {
		return mvp;
	}

	/**
	 * @return Projection Matrix
	 */
	const Matrix4f &getProjectionMatrix () const {
		return projectionMatrix;
	}

	/**
	 * @param projectionMatrix Projection Matrix
	 */
	void setProjectionMatrix (const Matrix4f &projectionMatrix) {
		this->projectionMatrix = projectionMatrix;
		mvp = projectionMatrix * viewMatrix * modelMatrix;
	}

	/**
	 * @return View Matrix
	 */
	const Matrix4f &getViewMatrix () const {
		return viewMatrix;
	}

	/**
	 * @param viewMatrix View Matrix
	 */
	void setViewMatrix (const Matrix4f &viewMatrix) {
		this->viewMatrix = viewMatrix;
		mvp = projectionMatrix * viewMatrix * modelMatrix;
	}

	/**
	 * @brief Updates the frame buffer size
	 * @param w Width
	 * @param h Height
	 */
	void updateFBSize (float w, float h) {
		FBWidth = w;
		FBHeight = h;
	}

	/**
	 * @brief Handle to the GFLW window for rendering
	 */
	void setWindow (GLFWwindow *w) {
		window = w;
	}

	/**
	 * @brief Sets the pointer to the Hmd
	 */
	void setHmd (ovrHmd h) {
		hmd = h;
	}

protected:

	std::shared_ptr<Mesh> mesh; ///< Bounded mesh
	std::shared_ptr<GLShader> shader; ///< Bounded shader
	std::unique_ptr<GLShader> bboxShader; ///< Bounding box shader
	float FBWidth, FBHeight; ///< To avoid cyclic includes and incomplete type errors
	GLFWwindow *window; ///< GFLW window handle
	ovrHmd hmd; ///< Head mounted device

private:

	Matrix4f viewMatrix; ///< View matrix
	Matrix4f modelMatrix; ///< Model matrix
	Matrix3f normalMatrix; ///< Transpose inverse model matrix
	Matrix4f projectionMatrix; ///< Projection matrix
	Matrix4f mvp; ///< MVP matrix
};

VR_NAMESPACE_END
