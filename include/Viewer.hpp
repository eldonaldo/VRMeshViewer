#pragma once

#include "common.hpp"
#include "mesh/Mesh.hpp"
#include "renderer/Renderer.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief Basis mesh viewer class.
 *
 * This class uses GLFW to construct a OS specific
 * window and allows all modern extension to be loaded with GLEW.
 *
 * The desired OpenGL context version can be specified in the commons.hpp
 * header file.
 */
class Viewer {
public:

	/**
	 * @brief Default constructor
	 */
	Viewer (const std::string &title, int width, int height, bool fullscreen = false) throw ();

	/**
	 * @brief Default constructor
	 */
	virtual ~Viewer ();

	/**
	 * Sets the bounded renderer.
	 *
	 * The Viewer uses this renderer to generate drawings defined with
	 * respect to the renderer class.
	 *
	 * @param r Bounded Renderer
	 */
	void setRenderer (std::unique_ptr<Renderer> &r) {
		renderer = std::move(r);
	}

	/**
	 * @brief Displays the data
	 *
	 * Internally it calls the Renderer::update() and Renderer::draw() methods. The purpose of
	 * Renderer::update() is that the states get updated and Renderer::draw() is responsible
	 * of drawing the data. Of course a renderer must be set in advance.
	 */
	virtual void display (std::shared_ptr<Mesh> &mesh) throw ();

	/**
	 * @brief Retrieve some OpenGL infos
	 *
	 * @return Supported OpenGL Versions
	 */
	std::string info ();

protected:


	/**
	 * @brief Displays the FPS count in the title of the window and returns the calculated FPS
	 */
	virtual void calcAndAppendFPS ();

protected:

	GLFWwindow *window; ///< GLFW window pointer
	const std::string title; ///< Window title
	int FBWidth, FBHeight; ///< Framebuffer size
	int width, height; ///< Window width and height
	Color3f background; ///< Background color
	const float interval; ///< Interval to refresh FPS
	unsigned int frameCount = 0; ///< Frame count
	unsigned int t0 = 0; ///< Initialization time
	double fps = 0.0; ///< FPS count
	bool appFPS = true; ///< If true, then the current FPS count is appended to the window title
	bool fullscreen; ///< Need fullscreen?
	std::unique_ptr<Renderer> renderer; ///< Bounded renderer
	Arcball arcball; ///< Arcball
	bool arcballActive = false; ///< Needed to determine when to use the arcball on a mouse button click
	Vector2i lastPos; ///< Last click position used for the arcball
};

VR_NAMESPACE_END
