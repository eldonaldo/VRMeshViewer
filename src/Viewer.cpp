#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

#if defined(WIN32)
	static bool glewInitialized = false;
#endif

Viewer::Viewer (const std::string &title, int width, int height, bool fullscreen) throw ()
	: title(title), width(width), height(height), fullscreen(fullscreen), interval(1.f) {

	// Initialize GLFW
	if (!glfwInit())
		throw VRException("Could not not start GFLW");

	// Request OpenGL compatible 3.3 context with the core profile enabled
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (fullscreen) {
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		window = glfwCreateWindow(mode->width, mode->height, title.c_str(), monitor, nullptr);
	} else {
		window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	}

	if (!window)
		throw VRException("Could not open a GFLW window");

	glfwMakeContextCurrent(window);

// Initialize GLEW
#if defined(PLATFORM_WINDOWS)
	if (!glewInitialized) {
		glewExperimental = GL_TRUE;
		glewInitialized = true;
		if (glewInit() != GLEW_NO_ERROR)
			throw VRException("Could not initialize GLEW!");
	}
#endif

	// Default view port and reset all pixels to black
	background = Vector3f(0.8f, 0.8f, 0.8f);
//	background = Vector3f(0.0f, 0.0f, 0.0f);

	glfwGetFramebufferSize(window, &FBWidth, &FBHeight);
	glViewport(0, 0, FBWidth, FBHeight);
	glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

#if defined(PLATFORM_APPLE)
    /* Poll for events once before starting a potentially
       lengthy loading process. This is needed to be
       classified as "interactive" by other software such
       as iTerm2 */

    glfwPollEvents();
#endif

	// Set callbacks
	glfwSetKeyCallback(window, [] (GLFWwindow *w, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(w, 1);
	});

	glfwSetWindowSizeCallback(window, [] (GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	});

	// Print OpenGL context info
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL version supported on this machine: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Acquired OpenGL version: " << glfwGetVersionString() << std::endl;
}

void Viewer::calcAndAppendFPS () {
	// Get the current time in seconds since the program started (non-static, so executed every time)
	double currentTime = glfwGetTime();

	// Calculate and display the FPS
	if ((currentTime - t0) > interval) {
		// Calculate the FPS as the number of frames divided by the interval in seconds
		fps = double(frameCount) / (currentTime - t0);

		// Append to window title
		std::string newTitle = title + " | FPS: " + toString(int(fps)) + " @ " + toString(width) + "x" + toString(height);
		glfwSetWindowTitle(window, newTitle.c_str());

		// Reset the FPS frame counter and set the initial time to be now
		frameCount = 0;
		t0 = glfwGetTime();
	} else {
		frameCount++;
	}
}

void Viewer::display (std::shared_ptr<Mesh> &mesh) throw () {
	if (!renderer)
		throw VRException("No renderer attached. Viewer can not display the object.");

	renderer->setMesh(mesh);
	renderer->preProcess();

	// Render loop
	t0 = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		// Clear
		glfwMakeContextCurrent(window);
		glfwGetWindowSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (appFPS)
			calcAndAppendFPS();

		renderer->update();
		renderer->draw();

		glfwSwapBuffers(window);
//		glfwWaitEvents();
		glfwPollEvents();
	}

	renderer->cleanUp();
}

Viewer::~Viewer () {
	glfwDestroyWindow(window);
	glfwTerminate();
}

VR_NAMESPACE_END
