#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

// Used inside callback functions to access the viewer's state
Viewer *__cbref;

#if defined(PLATFORM_WINDOWS)
	static bool glewInitialized = false;
#endif

Viewer::Viewer (const std::string &title, int width, int height, bool useRift, bool debug) throw ()
	: title(title), width(width), height(height), useRift(useRift), interval(1.f), lastPos(0, 0)
	, scaleMatrix(Matrix4f::Identity()), rotationMatrix(Matrix4f::Identity()), translateMatrix(Matrix4f::Identity())
	, debug(debug), hmd(nullptr) {

	// LibOVR need to be initialized before GLFW
	ovr_Initialize();
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
	//glfwWindowHint(GLFW_SAMPLES, 2);
	
	if (useRift) {
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		//window = glfwCreateWindow(mode->width, mode->height, title.c_str(), monitor, nullptr);
		window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
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
	glViewport(0, 0, width, height);
	glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	// Enable depth testing and multi sampling
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthFunc(GL_LEQUAL);
	//glEnable(GL_MULTISAMPLE);

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
		switch (key) {
			// Exit render loop
			case GLFW_KEY_ESCAPE:
				if (action == GLFW_PRESS)
					glfwSetWindowShouldClose(window, 1);
				break;

			// Recemter HDM pose
			case GLFW_KEY_R:

				ovrHmd_RecenterPose(__cbref->hmd);
				break;

			// Enable / disable v-sync
			case GLFW_KEY_V: {
				static bool disable = true;

				if (disable)
					ovrHmd_SetEnabledCaps(__cbref->hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction | ovrHmdCap_NoVSync);
				else
					ovrHmd_SetEnabledCaps(__cbref->hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

				disable = !disable;
				break;
			}
		}
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
		__cbref->scaleMatrix = scale(__cbref->scaleMatrix, 0.015f * y);
	});

	/* Window size callback */
	glfwSetWindowSizeCallback(window, [] (GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
		glfwGetFramebufferSize(window, &(__cbref->FBWidth), &(__cbref->FBHeight));
		__cbref->width = width; __cbref->height = height;
		if (__cbref->renderer)
			__cbref->renderer->updateFBSize(__cbref->FBWidth, __cbref->FBHeight);
	});

	// Set pointer for callback functions
	__cbref = this;

	// Leap hands
	hands[0] = std::make_shared<SkeletonHand>(true); // Right
	hands[1] = std::make_shared<SkeletonHand>(false); // Left

	// Enable HMD mode and pass through
	if (useRift)
		leapController.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>(Leap::Controller::PolicyFlag::POLICY_IMAGES | Leap::Controller::PolicyFlag::POLICY_OPTIMIZE_HMD));

	// Create gesture handler
	gestureHandler = std::make_shared<GestureHandler>();
	gestureHandler->setViewer(this);
}

void Viewer::calcAndAppendFPS () {
	static float t0 = glfwGetTime();

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

void Viewer::placeObject (std::shared_ptr<Mesh> &m) {
	BoundingBox3f &bbox = m->getBoundingBox();

	// Calculate current bounding box diagonal length
	float diag = (bbox.max - bbox.min).norm();
	float factor = Settings::getInstance().MESH_DIAGONAL / diag;

	// Translate to center
	Matrix4f T = translate(Matrix4f::Identity(), Vector3f(-bbox.getCenter().x(), -bbox.getCenter().y(), -bbox.getCenter().z()));

	// Compute scaling matrix
	Matrix4f S = scale(Matrix4f::Zero(), factor);

	// Transform object outside of OpenGL such that the correct metric units and center position are right away passed into OpenGL
	MatrixXf vertices = m->getVertexPositions();
	MatrixXf newPos(3, vertices.cols());
	Matrix4f transformMat = T * S;

	bbox.reset();
	for (int i = 0; i < vertices.cols(); i++) {
		Vector3f v = (transformMat * Vector4f(vertices.col(i).x(), vertices.col(i).y(), vertices.col(i).z(), 1.f)).head<3>();
		newPos.col(i) = v;
		bbox.expandBy(v);
	}

	m->setVertexPositions(newPos);
}

void Viewer::attachLeap (std::unique_ptr<LeapListener> &l) {
	leapListener = std::move(l);
	leapListener->setHands(hands[0], hands[1]);
	leapController.addListener(*leapListener);
}

void Viewer::display(std::shared_ptr<Mesh> &m, std::unique_ptr<Renderer> &r) throw () {
	renderer = std::move(r);
	mesh = m;

	// Reconfigure settings if the target is the Rift
	if (renderer->getClassType() == EHMDRenderer && hmd != nullptr) {
		width = hmd->Resolution.w;
		height = hmd->Resolution.h;
		glfwSetWindowSize(window, width, height);
		glfwGetFramebufferSize(window, &FBWidth, &FBHeight);
		glViewport(0, 0, width, height);
		if (leapListener != nullptr)
			leapListener->setSize(width, height, FBWidth, FBHeight);
	}

	// Share the HMD
	if (leapListener != nullptr) {
		leapListener->setHmd(hmd);
		leapListener->setGestureHandler(gestureHandler);
	}

	renderer->setController(leapController);
	renderer->setHmd(hmd); 

	// Place object in world for immersion
	placeObject(mesh);

	// Renderer pre processing
	gestureHandler->setMesh(mesh);
	renderer->setMesh(mesh);
	renderer->setHands(hands[0], hands[1]);
	renderer->setWindow(window);
	renderer->updateFBSize(FBWidth, FBHeight);
	renderer->preProcess();

	// Print some info
	std::cout << info() << std::endl;


	// Render loop
	glfwSwapInterval(0);
	while (!glfwWindowShouldClose(window)) {
		// Bind "the" framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Update arcball
		rotationMatrix = arcball.matrix(renderer->getViewMatrix());

		// Update state
		renderer->update(scaleMatrix, rotationMatrix, translateMatrix);

		// Clear buffers
		renderer->clear(background);

		// Draw using attached renderer
		renderer->draw();

		// Swap framebuffer, only if the rift is not attached
		if (renderer->getClassType() != EHMDRenderer)
			glfwSwapBuffers(window);

		// Poll or wait for events
		glfwPollEvents();

		if (appFPS)
			calcAndAppendFPS();
	}

	// Renderer cleapup
	renderer->cleanUp();
}

std::string Viewer::info () {
	return tfm::format(
		"Viewer[\n"
		"  Size = %s,\n"
		"  FBSize = %s,\n"
		"  Engine = %s,\n"
		"  OpenGL version supported on this machine = %s,\n"
		"  Acquired OpenGL version = %s,\n"
		"  Renderer = %s,\n"
		"  Mesh = %s\n"
		"]\n",
		toString(width) + " x " + toString(height),
		toString(FBWidth) + " x " + toString(FBHeight),
		glGetString(GL_RENDERER),
		glGetString(GL_VERSION),
		glfwGetVersionString(),
		indent((renderer ? renderer->info() : "null")),
		indent((mesh ? mesh->toString() : "null"))
	);
}

Arcball& Viewer::getArcball () {
	return arcball;
}

Matrix4f& Viewer::getScaleMatrix () {
	return scaleMatrix;
}

Matrix4f& Viewer::getTranslateMatrix () {
	return translateMatrix;
}

Matrix4f& Viewer::getRotationMatrix() {
	return rotationMatrix;
}

Vector2i& Viewer::getLastPos() {
	return lastPos;
}

void Viewer::setLastPos(Vector2i &v) {
	lastPos = v;
}

Viewer::~Viewer () {
	if (leapListener && leapListener != nullptr)
		leapController.removeListener(*leapListener);

	glfwDestroyWindow(window);
	glfwTerminate();

	// Destroy the rift. Needs to be called after glfwTerminate
	if (hmd) {
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
	}
}

VR_NAMESPACE_END
