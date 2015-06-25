#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

#if defined(PLATFORM_WINDOWS)
	static bool glewInitialized = false;
#endif

Viewer::Viewer(const std::string &title, int width, int height, bool fullscreen)
	: title(title), width(width), height(height), interval(1.f), lastPos(0, 0)
	, scaleMatrix(Matrix4f::Identity()), rotationMatrix(Matrix4f::Identity()), translateMatrix(Matrix4f::Identity())
	, hmd(nullptr), uploadAnnotation(false), loadAnnotationsFlag(false), sphereRadius(0.f), sequenceNr(0), netSocket(nullptr)
	, ready(false) {

	// Append networking mode in title
	if (Settings::getInstance().NETWORK_ENABLED) {
		std::string mode = Settings::getInstance().NETWORK_MODE == NETWORK_MODES::CLIENT ? "Client listening on " : "Server sending to ";
		this->title += " | " + mode + Settings::getInstance().NETWORK_IP + ":" + std::to_string(Settings::getInstance().NETWORK_PORT);
	}

	// LibOVR need to be initialized before GLFW
	if (Settings::getInstance().USE_RIFT) {
		ovr_Initialize();
		hmd = ovrHmd_Create(0);

		if (!hmd)
			hmd = ovrHmd_CreateDebug(ovrHmdType::ovrHmd_DK2);

		if (!hmd)
			throw VRException("Could not start the Rift");

		if (!ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0))
			throw VRException("The Rift does not support all of the necessary sensors");
	}

	// Init GLFW/Glew if rift used. Otherwise nanogui will do the job
	if (Settings::getInstance().USE_RIFT)
		init();

    // Setup arcball
    arcball.setSize(Vector2i(width, height));

	// Leap hands
	hands[0] = std::make_shared<SkeletonHand>(true); // Right
	hands[1] = std::make_shared<SkeletonHand>(false); // Left

	// Enable HMD mode and pass through
	if (Settings::getInstance().USE_RIFT)
		leapController.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>(Leap::Controller::PolicyFlag::POLICY_IMAGES | Leap::Controller::PolicyFlag::POLICY_OPTIMIZE_HMD));

	// Default leap listener
	std::unique_ptr<LeapListener> leap(new LeapListener(Settings::getInstance().USE_RIFT));
	attachLeap(leap);

	// Create gesture handler
	gestureHandler = std::make_shared<GestureHandler>();
	gestureHandler->setViewer(this);

	// Seed rnd generator
	srand(glfwGetTime());

	// Calc fps at startup
	if (!Settings::getInstance().USE_RIFT && appFPS)
		calcAndAppendFPS();
}

void Viewer::init () {
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

	if (Settings::getInstance().USE_RIFT || Settings::getInstance().FULLSCREEN) {
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		//viewerGLFWwindow = glfwCreateWindow(mode->width, mode->height, this->title.c_str(), monitor, nullptr);
		viewerGLFWwindow = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);
	} else {
		viewerGLFWwindow = glfwCreateWindow(width, height, this->title.c_str(), nullptr, nullptr);
	}

	if (!viewerGLFWwindow)
		throw VRException("Could not open a GFLW viewerGLFWwindow");

	glfwMakeContextCurrent(viewerGLFWwindow);

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

	glfwGetFramebufferSize(viewerGLFWwindow, &FBWidth, &FBHeight);
	glViewport(0, 0, width, height);
	glClearColor(background.coeff(0), background.coeff(1), background.coeff(2), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(viewerGLFWwindow);

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
}

void Viewer::calcAndAppendFPS () {
	static float t0 = glfwGetTime();

	// Get the current time in seconds since the program started (non-static, so executed every time)
	double currentTime = glfwGetTime();

	// Calculate and display the FPS
	if ((currentTime - t0) > interval) {
		// Calculate the FPS as the number of frames divided by the interval in seconds
		fps = double(frameCount) / (currentTime - t0);

		// Append to viewerGLFWwindow title
		std::string newTitle = title + " | FPS: " + toString(int(fps)) + " @ " + toString(width) + "x" + toString(height);
		glfwSetWindowTitle(viewerGLFWwindow, newTitle.c_str());
		
		// Reset the FPS frame counter and set the initial time to be now
		frameCount = 0;
		t0 = glfwGetTime();
	} else {
		frameCount++;
	}
}

void Viewer::placeObjectAndBuildKDTree (std::shared_ptr<Mesh> &m) {
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

	KDTree &kdtree = m->getKDTree();

	kdtree.clear();
	kdtree.reserve(sizeof(Vector3f) * vertices.cols());
	bbox.reset();

	for (int i = 0; i < vertices.cols(); i++) {
		Vector3f v = (transformMat * Vector4f(vertices.col(i).x(), vertices.col(i).y(), vertices.col(i).z(), 1.f)).head<3>();
		newPos.col(i) = v;
		bbox.expandBy(v);

		GenericKDTreeNode<Point3f, Point3f> kdPoint(v, m->getVertexNormals().col(i));
		kdtree.push_back(kdPoint);
	}

	m->setVertexPositions(newPos);

	// Bounding sphere
	sphereCenter = mesh->getBoundingBox().getCenter();
	sphereRadius = (mesh->getBoundingBox().min - mesh->getBoundingBox().max).norm() * 0.5f;

	// Build kd-tree
	kdtree.build();
}

void Viewer::attachLeap (std::unique_ptr<LeapListener> &l) {
	leapListener = std::move(l);
	leapListener->setHands(hands[0], hands[1]);
}

void Viewer::display(std::shared_ptr<Mesh> &m, std::shared_ptr<Renderer> &r) {
	renderer = r;
	mesh = m;

	// Reconfigure settings if the target is the Rift
	if (renderer->getClassType() == EHMDRenderer && hmd != nullptr) {
		width = hmd->Resolution.w;
		height = hmd->Resolution.h;
		glfwSetWindowSize(viewerGLFWwindow, width, height);
		glfwGetFramebufferSize(viewerGLFWwindow, &FBWidth, &FBHeight);
		glViewport(0, 0, width, height);
	}

	renderer->setController(leapController);
	renderer->setHmd(hmd); 

	// Place object in world for immersion
	placeObjectAndBuildKDTree(mesh);

	// Renderer pre processing
	gestureHandler->setMesh(mesh);
	renderer->setMesh(mesh);
	renderer->setHands(hands[0], hands[1]);
	renderer->setWindow(viewerGLFWwindow);
	renderer->updateFBSize(FBWidth, FBHeight);
	renderer->preProcess();

	// Share the HMD
	leapListener->setHmd(hmd);
	leapListener->setMesh(mesh);
	leapListener->setGestureHandler(gestureHandler);
	if (Settings::getInstance().LEAP_USE_LISTENER)
		leapController.addListener(*leapListener);

	// Load annotations if desired 
	if (loadAnnotationsFlag)
		loadAnnotationsOnLoop(); 

	// Print some info
	std::cout << info() << std::endl;

	// ready to display opengl content
	ready = true;
}

void Viewer::render (long lastTime) {
	// Bind the default framebuffer
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

	// Calc fps
	if (!Settings::getInstance().USE_RIFT && appFPS)
		calcAndAppendFPS();

	// Add annotation
	if (uploadAnnotation) {
		addAnnotation(annotationTarget, annotationNormal);
		uploadAnnotation = false;
	}

//	// Swap framebuffer, only if the rift is not attached
//	if (renderer->getClassType() != EHMDRenderer)
//		glfwSwapBuffers(viewerGLFWwindow);

	// Networking
	if (Settings::getInstance().NETWORK_ENABLED && (long(glfwGetTime() * 1000) - lastTime) >= Settings::getInstance().NETWORK_SEND_RATE) {
		processNetworking();
		lastTime = long(glfwGetTime() * 1000);
	}
}

void Viewer::renderLoop () {
	// Last send time in milliseconds
	long lastTime = glfwGetTime() * 1000;

	// Render loop
	glfwSwapInterval(0);
	while (!glfwWindowShouldClose(viewerGLFWwindow)) {
		// Poll for events to process
		glfwPollEvents();

		// Render
		if (ready)
			render(lastTime);
	}

	// Renderer cleapup
	renderer->cleanUp();
}

void Viewer::processNetworking () {
	if (Settings::getInstance().NETWORK_MODE == NETWORK_MODES::SERVER && Settings::getInstance().NETWORK_NEW_DATA)  {
		netSocket->send(serializeTransformationState(), Settings::getInstance().NETWORK_IP, Settings::getInstance().NETWORK_PORT);

		// Increase sequence nr
		sequenceNr = (sequenceNr + 1) % std::numeric_limits<long>::max();
	} else if (Settings::getInstance().NETWORK_LISTEN) {
		netSocket->receive();
	}

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
			/*else if (prefix == "view")
				renderer->setViewMatrix(stringToMatrix4f(content));*/
			/*else if (prefix == "translate")
				translateMatrix = stringToMatrix4f(content);*/
			else if (prefix == "annotations_new") {
				content = payload.substr(payload.find_first_of('{') + 1);
				content = content.substr(0, content.find_last_of('}'));
				loadAnnotationsFromString(content);
			} else if (prefix == "annotations_delete") {
				content = payload.substr(payload.find_first_of('{') + 1);
				content = content.substr(0, content.find_last_of('}'));
				std::vector<Pin> toDelete = getAnnotationsFromString(content);
				for (auto &pDelete : toDelete) {
					for (auto iter = pinList.begin(); iter != pinList.end(); iter++) {
						if (**iter == pDelete) {
							(*iter)->releaseBuffers();
							pinList.erase(iter);
							break;
						}
					}
				}
			}
		}
	}
}

bool Viewer::deletePinIfHit(Vector3f &position) {
	bool found = false;
	for (auto iter = pinList.begin(); iter != pinList.end(); iter++) {
		BoundingBox3f bbox = (*iter)->getBoundingBox();
		if (bbox.contains(position)) {

			// Copy pin and inform the client
			Pin copy((*iter)->getPosition(), (*iter)->getNormal(), mesh->getNormalMatrix());
			pinListDelete.push_back(copy);

			// Clear graphics memory and delete it safely
			(*iter)->releaseBuffers();
			pinList.erase(iter);
			found = true;

			// Need to send a new packet
			Settings::getInstance().NETWORK_NEW_DATA = true;
			break;
		}
	}

	return found;
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
	
	// Pins to add
	if (!pinListAdd.empty()) {
		state += "annotations_new {\n";
		state += serializeAnnotations(pinListAdd);
		state += "}";
	}

	// Pins to delete
	if (!pinListDelete.empty()) {
		state += "annotations_delete {\n";
		state += serializeAnnotations(pinListDelete);
		state += "}";
	}

	// Clear the lists for the next packet
	pinListAdd.clear();
	pinListDelete.clear();

	return state;
}

void Viewer::attachSocket(UDPSocket &s) {
	netSocket = &s;
}

std::string Viewer::serializeAnnotations(std::vector<Pin> &list) {
	std::string	output;
	for (auto &p : list)
		output += p.serialize();
	return output;
}

std::string Viewer::serializeAnnotations(std::vector<std::shared_ptr<Pin>> &list) {
	std::string	output;
	for (auto &p : list)
		output += p->serialize();
	return output;
}

void Viewer::loadAnnotations(const std::string &s) {
	loadAnnotationsFlag = true;
	annotationsLoadPath = s;
}

std::vector<Pin> Viewer::getAnnotationsFromString(std::string &s) {
	std::vector<Pin> list;
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

			Matrix3f nm = mesh->getNormalMatrix();
			Pin pin(position, normal, nm);
			pin.setColor(color);
			list.push_back(pin);
		}
	}

	return list;
}

void Viewer::loadAnnotationsFromString(std::string &s) {
	std::vector<Pin> list = getAnnotationsFromString(s);
	for (auto &p : list)
		addAnnotation(p.getPosition(), p.getNormal(), p.getColor());
}

bool Viewer::pinListContains(std::vector<std::shared_ptr<Pin>> &list, const Pin &p) const {
	for (auto &pin : list) 
		if (*pin == p)
			return true;
	
	return false;
}

void Viewer::addAnnotation(const Vector3f &pos, const Vector3f &n, const Vector3f &c) {
	if (mesh == nullptr)
		throw VRException("No mesh to add annotations");

	Matrix3f nm = mesh->getNormalMatrix();
	std::shared_ptr<Pin> pin = std::make_shared<Pin>(pos, n, nm);
	pin->setColor(c);

	if (!pinListContains(pinList, *pin)) {
		pinList.push_back(pin);
		renderer->uploadAnnotation(pin, pinList);

		// Need to send a new packet
		Settings::getInstance().NETWORK_NEW_DATA = true;
		
		// Inform the client that a new pin needs to be added
		Pin copy(pos, n, nm);
		copy.setColor(c);
		pinListAdd.push_back(copy);
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
		file << serializeAnnotations(pinList);
		file.close();

		cout << "Saved to: " << savePath << endl;
	}
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

std::shared_ptr<Renderer> &Viewer::getRenderer () {
	return renderer;
}

Viewer::~Viewer () {
	// Remove leap listeners if any
	if (Settings::getInstance().LEAP_USE_LISTENER)
		leapController.removeListener(*leapListener);

	glfwDestroyWindow(viewerGLFWwindow);
	glfwTerminate();

	// Destroy the rift. Needs to be called after glfwTerminate
	if (hmd) {
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
	}
}

VR_NAMESPACE_END
