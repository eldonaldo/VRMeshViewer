#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

// Used inside callback functions to access the viewer's state
Viewer *__cbref;

#if defined(PLATFORM_WINDOWS)
	static bool glewInitialized = false;
#endif

Viewer::Viewer (const std::string &title, int width, int height, bool fullscreen, bool debug) throw ()
	: title(title), width(width), height(height), fullscreen(fullscreen), interval(1.f), lastPos(0, 0), scaleMatrix(Matrix4f::Identity()), debug(debug) {

	// LibOVR need to be initialized before GLFW
	ovr_Initialize();

	if (!debug)
		hmd = ovrHmd_Create(0);
	
	if (!hmd)
		hmd = ovrHmd_CreateDebug(ovrHmdType::ovrHmd_DK2);
		
	if (!hmd)
		throw VRException("Could not start the Rift");

	if (!ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0))
		throw VRException("The Rift does not support all of the necessary sensors");

	// Initialize GLFW
	if (!glfwInit())
		throw VRException("Could not start GLFW");

	// Request OpenGL compatible 3.3 context with the core profile enabled
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Enable multi sampling
	glfwWindowHint(GLFW_SAMPLES, 2);

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

	glfwGetFramebufferSize(window, &FBWidth, &FBHeight);
	glViewport(0, 0, FBWidth, FBHeight);
	glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);

	// Enable depth testing and multi sampling
//	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

#if defined(PLATFORM_APPLE)
    /* Poll for events once before starting a potentially
       lengthy loading process. This is needed to be
       classified as "interactive" by other software such
       as iTerm2 */

    glfwPollEvents();
#endif


    // Setup arcball
    arcball.setSize(Vector2i(width, height));

	// Set callbacks
	glfwSetKeyCallback(window, [] (GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, 1);
	});

	/* Mouse click callback */
	glfwSetMouseButtonCallback(window, [] (GLFWwindow *window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			__cbref->arcball.button(__cbref->lastPos, action == GLFW_PRESS);
	});

	/* Mouse movement callback */
	glfwSetCursorPosCallback(window, [] (GLFWwindow *window, double x, double y) {
		__cbref->lastPos = Vector2i(int(x), int(y));
		__cbref->arcball.motion(__cbref->lastPos);
	});

	/* Mouse wheel callback */
	glfwSetScrollCallback(window, [] (GLFWwindow *window, double x, double y) {
		__cbref->scaleMatrix += Matrix4f::Identity() * 0.2f * y;
		__cbref->scaleMatrix(3, 3) = 1.f;

		if (__cbref->scaleMatrix(0, 0) <= 0)
			__cbref->scaleMatrix = Matrix4f::Zero();
	});

	/* Window size callback */
	glfwSetWindowSizeCallback(window, [] (GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
		glfwGetFramebufferSize(window, &(__cbref->FBWidth), &(__cbref->FBHeight));
		__cbref->width = width; __cbref->height = height;
		if (__cbref->renderer)
			__cbref->renderer->updateFBSize(__cbref->FBWidth, __cbref->FBHeight);
	});

	// Set reference for callback functions
	__cbref = this;
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

	// Renderer pre processing
	renderer->setHmd(hmd);
	renderer->setMesh(mesh);
	renderer->setWindow(window);
	renderer->updateFBSize(FBWidth, FBHeight);
	renderer->preProcess();

	// Print some info
	std::cout << info() << std::endl;

	// Init time t0 for FPS calculation
	t0 = glfwGetTime();

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		if (appFPS)
			calcAndAppendFPS();

		// Arcball rotationa and scaling
		renderer->setModelMatrix(scaleMatrix * arcball.matrix(renderer->getViewMatrix()));

		// Update state
		renderer->update();

		// Clear buffers
		renderer->clear(background);

		// Draw using attached renderer
		renderer->draw();

		// Swap framebuffer
		glfwSwapBuffers(window);

		// Poll or wait for events
		glfwPollEvents();
		//glfwWaitEvents();
	}

	// Renderer cleapup
	renderer->cleanUp();
}

Viewer::~Viewer () {
	glfwDestroyWindow(window);
	glfwTerminate();

	// Destroy the rift. Needs to be called after glfwTerminate
	if (hmd) {
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
	}
}

VR_NAMESPACE_END
