#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

// Used inside callback functions to access the viewer's state
Viewer *__cbref;

#if defined(PLATFORM_WINDOWS)
	static bool glewInitialized = false;
#endif

Viewer::Viewer(const std::string &title, int width, int height, bool fullscreen)
	: title(title), width(width), height(height), interval(1.f), lastPos(0, 0)
	, scaleMatrix(Matrix4f::Identity()), rotationMatrix(Matrix4f::Identity()), translateMatrix(Matrix4f::Identity())
	, hmd(nullptr), uploadAnnotation(false), loadAnnotationsFlag(false), sphereRadius(0.f), sequenceNr(0), netSocket(nullptr) {

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
	if (!Settings::getInstance().USE_RIFT)
		glfwWindowHint(GLFW_SAMPLES, 4);

	if (Settings::getInstance().USE_RIFT || fullscreen) {
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

	// Enable or disable OpenGL features
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (!Settings::getInstance().USE_RIFT)
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
		switch (key) {
			// Exit render loop
			case GLFW_KEY_ESCAPE:
				if (action == GLFW_PRESS)
					glfwSetWindowShouldClose(window, 1);
				break;

			// Recenter HDM pose
			case GLFW_KEY_R:
				if (action == GLFW_PRESS)
					ovrHmd_RecenterPose(__cbref->hmd);
				break;

			// Enable / disable v-sync
			case GLFW_KEY_V: {
				static bool disable = true;
				if (action == GLFW_PRESS) {
					if (disable)
						ovrHmd_SetEnabledCaps(__cbref->hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction | ovrHmdCap_NoVSync);
					else
						ovrHmd_SetEnabledCaps(__cbref->hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
					disable = !disable;
				}
				break;
			}

			// Draw wireframe overlay
			case GLFW_KEY_W: {
				static bool disable = true;
				if (action == GLFW_PRESS) {
					Settings::getInstance().MESH_DRAW_WIREFRAME = disable;
					disable = !disable;
				}
				break;
			}

			// Draw wireframe overlay
			case GLFW_KEY_B: {
				static bool disable = true;
				if (action == GLFW_PRESS) {
					Settings::getInstance().MESH_DRAW_BBOX = disable;
					disable = !disable;
				}
				break;
			}

			// Draw mesh or not?
			case GLFW_KEY_M: {
				static bool disable = false;
				if (action == GLFW_PRESS) {
					Settings::getInstance().MESH_DRAW = disable;
					disable = !disable;
				}
				break;
			}

			// Show sphere or not
			case GLFW_KEY_S: {
				static bool disable = false;
				if (action == GLFW_PRESS) {
					Settings::getInstance().ENABLE_SPHERE = disable;
					disable = !disable;
				}
				break;
			}

			// Show virtual hands or not
			case GLFW_KEY_H: {
				static bool disable = false;
				if (action == GLFW_PRESS) {
					Settings::getInstance().SHOW_HANDS = disable;
					disable = !disable;
				}
				break;
			}

			// Enable/disable passthrough
			case GLFW_KEY_P: {
				static bool disable = false;
				if (action == GLFW_PRESS) {
					Settings::getInstance().LEAP_USE_PASSTHROUGH = disable;
					disable = !disable;
				}
				break;
			}

			// Place object to defauls
			case GLFW_KEY_C: {
				 __cbref->getTranslateMatrix() = Matrix4f::Identity();
				 __cbref->getScaleMatrix() = Matrix4f::Identity();
				 __cbref->getTranslateMatrix() = Matrix4f::Identity();
				break;
			}

			// Save annotations to a file
			case GLFW_KEY_A: {
				if (action == GLFW_PRESS)
					__cbref->saveAnnotations();

				break;
			}

			// Enable/disable leap for 2d use
			case GLFW_KEY_L: {
				static bool disable = true;
				if (action == GLFW_PRESS) {
					Settings::getInstance().USE_LEAP = disable && __cbref->leapController.isConnected();
					disable = !disable;
				}

				break;
			}
		}
	});

	/* Mouse click callback */
	glfwSetMouseButtonCallback(window, [] (GLFWwindow *window, int button, int action, int mods) {
		if (!Settings::getInstance().USE_RIFT && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && action == GLFW_PRESS) {
			// Query viewport
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			Vector2i viewPortSize(viewport[2], viewport[3]);

			// Query cursor position and depth value at this position
			double x, y; GLfloat z;
			glfwGetCursorPos(window, &x, &y);
			glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
			Vector3f pos(x, viewport[3] - y, z);

			// Unproject scene
			Matrix4f VM = __cbref->getRenderer()->getViewMatrix() * __cbref->getMesh()->getModelMatrix();
			Vector3f worldPosition = unproject(pos, VM, __cbref->getRenderer()->getProjectionMatrix(), viewPortSize);

			// Add an annotation
			Vector3f n(0.f, 1.f, 0.f);
			__cbref->addAnnotation(worldPosition, n);

//			cout << z << endl;
//			ppv(worldPosition);
		} else if (button == GLFW_MOUSE_BUTTON_LEFT) {
			__cbref->arcball.button(__cbref->lastPos, action == GLFW_PRESS);
		}
	});

	/* Mouse movement callback */
	glfwSetCursorPosCallback(window, [] (GLFWwindow *window, double x, double y) {
		__cbref->lastPos = Vector2i(int(x), int(y));
		__cbref->arcball.motion(__cbref->lastPos);
	});

	/* Mouse wheel callback */
	glfwSetScrollCallback(window, [] (GLFWwindow *window, double x, double y) {
		if (Settings::getInstance().NETWORK_ENABLED && Settings::getInstance().NETWORK_MODE == NETWORK_MODES::CLIENT)
			return;

		float scaleFactor = 0.05f;
#if defined(PLATFORM_WINDOWS)
		scaleFactor = 0.45f;
#endif
		if (y >= 0)
			__cbref->scaleMatrix = scale(__cbref->scaleMatrix, 1.f + scaleFactor);
		else
			__cbref->scaleMatrix = scale(__cbref->scaleMatrix, 1.f - scaleFactor);
	});

	/* Window size callback */
	glfwSetWindowSizeCallback(window, [] (GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
		glfwGetFramebufferSize(window, &(__cbref->FBWidth), &(__cbref->FBHeight));
		__cbref->width = width; __cbref->height = height;
		__cbref->getArcball().setSize(Vector2i(width, height));
		if (__cbref->renderer)
			__cbref->renderer->updateFBSize(__cbref->FBWidth, __cbref->FBHeight);
	});

	// Set pointer for callback functions
	__cbref = this;

	// Leap hands
	hands[0] = std::make_shared<SkeletonHand>(true); // Right
	hands[1] = std::make_shared<SkeletonHand>(false); // Left

	// Enable HMD mode and pass through
	if (Settings::getInstance().USE_RIFT)
		leapController.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>(Leap::Controller::PolicyFlag::POLICY_IMAGES | Leap::Controller::PolicyFlag::POLICY_OPTIMIZE_HMD));

	// Create gesture handler
	gestureHandler = std::make_shared<GestureHandler>();
	gestureHandler->setViewer(this);

	// Seed rnd generator
	srand(glfwGetTime());
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
	Matrix4f S = scale(Matrix4f::Identity(), factor);

	// Transform object outside of OpenGL such that the correct metric units and center position are right away passed into OpenGL
	MatrixXf vertices = m->getVertexPositions();
	MatrixXf newPos(3, vertices.cols());
	Matrix4f transformMat = S * T;

	bbox.reset();
	for (int i = 0; i < vertices.cols(); i++) {
		Vector3f v = (transformMat * Vector4f(vertices.col(i).x(), vertices.col(i).y(), vertices.col(i).z(), 1.f)).head<3>();
		newPos.col(i) = v;
		bbox.expandBy(v);
	}

	m->setVertexPositions(newPos);

	// Bounding sphere
	sphereCenter = mesh->getBoundingBox().getCenter();
	sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * 0.5f;
}

void Viewer::attachLeap (std::unique_ptr<LeapListener> &l) {
	leapListener = std::move(l);
	leapListener->setHands(hands[0], hands[1]);
}

void Viewer::display(std::shared_ptr<Mesh> &m, std::unique_ptr<Renderer> &r) {
	renderer = std::move(r);
	mesh = m;

	// Reconfigure settings if the target is the Rift
	if (renderer->getClassType() == EHMDRenderer && hmd != nullptr) {
		width = hmd->Resolution.w;
		height = hmd->Resolution.h;
		glfwSetWindowSize(window, width, height);
		glfwGetFramebufferSize(window, &FBWidth, &FBHeight);
		glViewport(0, 0, width, height);
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

	// Share the HMD
	if (leapListener != nullptr) {
		leapListener->setHmd(hmd);
		leapListener->setMesh(mesh);
		leapListener->setGestureHandler(gestureHandler);
		if (Settings::getInstance().LEAP_USE_LISTENER)
			leapController.addListener(*leapListener);
	}

	// Load annotations if desired 
	if (loadAnnotationsFlag)
		loadAnnotationsOnLoop(); 

	// Print some info
	std::cout << info() << std::endl;

	// Last send time in milliseconds
	long lastTime = glfwGetTime() * 1000;

	// Render loop
	glfwSwapInterval(0);
	while (!glfwWindowShouldClose(window)) {
		// Bind "the" framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Get a new leap frame if no listener is used
		if (!Settings::getInstance().LEAP_USE_LISTENER) {
			frame = leapController.frame();
			renderer->setFrame(frame);
			leapListener->onDirectFrame(frame);
		}

		// Update arcball
		if ((Settings::getInstance().NETWORK_ENABLED && !Settings::getInstance().USE_RIFT && Settings::getInstance().NETWORK_MODE == NETWORK_MODES::SERVER) ||
			(!Settings::getInstance().NETWORK_ENABLED && !Settings::getInstance().USE_RIFT && !Settings::getInstance().USE_LEAP)) {
			rotationMatrix = arcball.matrix(renderer->getViewMatrix());
		}

		// Bounding sphere
		renderer->setSphereCenter(sphereCenter);
		renderer->setSphereRadius(sphereRadius);

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

		// Calc fps
		if (!Settings::getInstance().USE_RIFT && appFPS)
			calcAndAppendFPS();
		
		// Add annotation
		if (uploadAnnotation) {
			addAnnotation(annotationTarget, annotationNormal);
			uploadAnnotation = false;
		}

		// Networking
		if (Settings::getInstance().NETWORK_ENABLED && (long(glfwGetTime() * 1000) - lastTime) >= Settings::getInstance().NETWORK_SEND_RATE) {
			processNetworking();
			lastTime = long(glfwGetTime() * 1000);
		}
	}
	
	// Renderer cleapup
	renderer->cleanUp();
	
	// Remove leap listeners if any
	if (leapListener != nullptr && Settings::getInstance().LEAP_USE_LISTENER) {
		leapController.removeListener(*leapListener);
		leapListener.release();
	}
}

void Viewer::processNetworking () {
	if (Settings::getInstance().NETWORK_MODE == NETWORK_MODES::SERVER)
		netSocket->send(serializeTransformationState(), Settings::getInstance().NETWORK_IP, Settings::getInstance().NETWORK_PORT);
	else if (Settings::getInstance().NETWORK_LISTEN)
		netSocket->receive();

	// Parse package and adjust model/view matrix if in client mode
	if (Settings::getInstance().NETWORK_MODE == NETWORK_MODES::CLIENT && netSocket->hasNewData()) {
		std::string payload = netSocket->getBufferContent();
		std::istringstream ss(payload);
		std::string line;

		// Parse packet
		while (std::getline(ss, line)) {
			std::istringstream lss(line);
			std::string prefix;
			lss >> prefix;
			std::string content = line.substr(line.find_first_of(' ') + 1);

			if (prefix == "scale")
				scaleMatrix = stringToMatrix4f(content);
			else if (prefix == "rotate")
				rotationMatrix = stringToMatrix4f(content);
			else if (prefix == "view")
				renderer->setViewMatrix(stringToMatrix4f(content));
			/*else if (prefix == "translate")
				translateMatrix = stringToMatrix4f(content);*/
			else if (prefix == "annotations") {
				content = payload.substr(payload.find_first_of('{') + 1);
				content = content.substr(0, content.find_last_of('}'));
				cout << content << endl;
				loadAnnotationsFromString(content);
			}
		}
	}

	// Increase sequence nr
	sequenceNr = (sequenceNr + 1) % std::numeric_limits<long>::max();
}

std::string Viewer::serializeTransformationState () {
	Matrix4f vm = renderer->getViewMatrix();

	// Matrices are stored row major
	std::string state;
	state += "id " + std::to_string(sequenceNr) + "\n";
	state += "translate " + matrix4fToString(translateMatrix) + "\n";
	state += "scale " + matrix4fToString(scaleMatrix) + "\n";
	state += "rotate " + matrix4fToString(rotationMatrix) + "\n";
	state += "view " + matrix4fToString(vm) + "\n";
	if (!pinList.empty()) {
		state += "annotations {\n";
		state += serializeAnnotations();
		state += "}";
	}
	return state;
}

void Viewer::attachSocket(UDPSocket &s) {
	netSocket = &s;
}

std::string Viewer::serializeAnnotations() {
	std::string	output;
	for (auto &p : pinList)
		output += p->serialize();
	return output;
}

void Viewer::loadAnnotations(const std::string &s) {
	loadAnnotationsFlag = true;
	annotationsLoadPath = s;
}

void Viewer::loadAnnotationsFromString(std::string &s) {
	pinList.clear();
	std::istringstream is(s);
	std::string line_str;
	while (std::getline(is, line_str)) {
		std::istringstream line(line_str);
		std::string prefix;
		line >> prefix;

		if (prefix == "pin") {
			Vector3f position, normal, color;
			while (std::getline(is, line_str)) {
				std::istringstream pinLine(line_str);
				std::string pinPrefix;
				pinLine >> pinPrefix;

				if (pinPrefix == "position")
					pinLine >> position.x() >> position.y() >> position.z();
				else if (pinPrefix == "normal")
					pinLine >> normal.x() >> normal.y() >> normal.z();
				else if (pinPrefix == "color")
					pinLine >> color.x() >> color.y() >> color.z();
				else
					break;
			}

			addAnnotation(position, normal, color);
		}
	}
}

void Viewer::loadAnnotationsOnLoop() {
	if (!fileExists(annotationsLoadPath))
		throw VRException("File \"%s\" does not exists!", annotationsLoadPath);

	std::ifstream is(annotationsLoadPath);
	if (is.fail())
		throw VRException("Unable to open file \"%s\"!", annotationsLoadPath);
	
	std::stringstream buffer;
	buffer << is.rdbuf();
	std::string annotations = buffer.str();
	loadAnnotationsFromString(annotations);
}

void Viewer::saveAnnotations () {
	if (!pinList.empty()) {
		std::ofstream file;
		std::size_t pos = mesh->getName().find_last_of("/\\");
		std::string path = mesh->getName().substr(0, pos);
		std::string filename = mesh->getName().substr(pos + 1);
		std::size_t pos1 = filename.find_last_of('.');
		filename = filename.substr(0, pos1) + "-annotations";

		int i = 1;
		std::string savePath = path + "/" + filename + "-" + toString(i) + ".txt";
		while (fileExists(savePath)) {
			i++;
			savePath = path + "/" + filename + "-" + toString(i) + ".txt";
		}

		file.open(savePath);
		file << serializeAnnotations();
		file.close();

		cout << "Saved to: " << savePath << endl;
	}
}

void Viewer::addAnnotation(Vector3f &pos, Vector3f &n, Vector3f &c) {
	if (mesh == nullptr)
		throw VRException("No mesh to add annotations");

	Matrix3f nm = mesh->getNormalMatrix();
	std::shared_ptr<Pin> pin = std::make_shared<Pin>(pos, n, nm);
	pin->setColor(c);

	pinList.push_back(pin);
	renderer->uploadAnnotation(pin, pinList);
}

void Viewer::addAnnotation(Vector3f &pos, Vector3f &n) {
	Vector3f randomColor(((float)rand() / (RAND_MAX)), ((float)rand() / (RAND_MAX)), ((float)rand() / (RAND_MAX)));
	addAnnotation(pos, n, randomColor);
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

std::shared_ptr<Mesh> &Viewer::getMesh() {
	return mesh;
}

std::vector<std::shared_ptr<Pin>> &Viewer::getAnnotations() {
	return pinList;
}

std::unique_ptr<Renderer> &Viewer::getRenderer () {
	return renderer;
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
