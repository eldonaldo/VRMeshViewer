#include "common.hpp"
#include "Viewer.hpp"
#include "mesh/WavefrontObj.hpp"
#include "renderer/PerspectiveRenderer.hpp"
#include "renderer/RiftRenderer.hpp"
#include "leap/LeapListener.hpp"
#include "network/UDPSocket.hpp"
#include <thread>

using namespace VR_NS;

void initShader(std::shared_ptr<GLShader> &shader);

bool parseArgs (int argc, char *argv[]) {
	if (argc < 2)
		return false;

	Settings::getInstance().USE_RIFT = Settings::getInstance().USE_LEAP = (std::string(argv[1]) == "3d" ? true : false);
	Settings::getInstance().MODEL = std::string(argv[2]);

	if (argc > 3)
		Settings::getInstance().ANNOTATIONS = std::string(argv[3]);

	// Networking args
	Settings::getInstance().NETWORK_ENABLED = false;
	if (argc >= 6) {
		Settings::getInstance().NETWORK_ENABLED = true;
		Settings::getInstance().NETWORK_MODE = NETWORK_MODES::CLIENT;
		Settings::getInstance().NETWORK_PORT = std::atoi(argv[5]);
		if (std::string(argv[4]) == "server") {
			Settings::getInstance().NETWORK_MODE = NETWORK_MODES::SERVER;
			Settings::getInstance().NETWORK_IP = std::string(argv[6]);
		}
	}

	return true;
}

int main (int argc, char *argv[]) {

	// LibOVR need to be initialized before GLFW
	ovr_Initialize();
	ovrHmd hmd = ovrHmd_Create(0);

	if (!hmd)
		hmd = ovrHmd_CreateDebug(ovrHmdType::ovrHmd_DK2);

	if (!hmd)
		throw std::runtime_error("Could not start the Rift");

	if (!ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0))
		throw std::runtime_error("The Rift does not support all of the necessary sensors");

	// Args
	if (!parseArgs(argc, argv)) {
		std::cout << "Usage: VRMeshViewer <3d|2d> <model.obj> [<none|annotations.txt>] [<client|server> <UDP-port> <ip-address>]" << std::endl;
		return -1;
	}

	// Settings
	int &width = Settings::getInstance().WINDOW_WIDTH, &height = Settings::getInstance().WINDOW_HEIGHT;
	float &fov = Settings::getInstance().FOV, &zNear = Settings::getInstance().Z_NEAR, &zFar = Settings::getInstance().Z_FAR;

	try {

		// This sets up the OpenGL context and needs the be first call
		Viewer viewer("Virtual Reality Mesh Viewer", width, height, hmd);

		// Create shader
		std::shared_ptr<GLShader> shader = std::make_shared<GLShader>();
		initShader(shader);

		// Create an appropriate renderer
		std::unique_ptr<Renderer> renderer;
		if (Settings::getInstance().USE_RIFT)
			renderer = std::unique_ptr<Renderer>(new RiftRenderer(shader, fov, width, height, zNear, zFar));
		else
			renderer = std::unique_ptr<Renderer>(new PerspectiveRenderer(shader, fov, width, height, zNear, zFar));

		// Load mesh
//		Settings::getInstance().MODEL = "resources/models/dragon/dragon.obj";
//		Settings::getInstance().MODEL = "resources/models/ironman/ironman.obj";
//		Settings::getInstance().MODEL = "resources/models/muro/muro.obj";
//		Settings::getInstance().MODEL = "C:/Users/pnico/Downloads/Ajax_Jotero_com.obj";
		std::shared_ptr<Mesh> mesh = std::make_shared<WavefrontOBJ>(Settings::getInstance().MODEL);

		// Create Leap listener
		std::unique_ptr<LeapListener> leap(new LeapListener());
		viewer.attachLeap(leap);

		// Networking
		std::unique_ptr<std::thread> netThread;
		asio::io_service io_service;
		asio::io_service::work work(io_service);

		/**
		 * If we're the server we need to choose another port.
		 * This is for debugging purposes only to run the application twice
		 * on the same machine and connect them together.
		 */
		short listenPort = Settings::getInstance().NETWORK_PORT;
		if (Settings::getInstance().NETWORK_MODE == NETWORK_MODES::SERVER)
			listenPort = Settings::getInstance().NETWORK_PORT - 1;
		UDPSocket socket(io_service, listenPort);

		// Setup networking
		if (Settings::getInstance().NETWORK_ENABLED) {

			// Print info
			std::cout << "Network[\n" <<
			"  Mode: " << (Settings::getInstance().NETWORK_MODE == 0 ? "Server" : "Client") << ",\n" <<
			"  Endpoint: " << Settings::getInstance().NETWORK_IP << ":" << Settings::getInstance().NETWORK_PORT << "\n" <<
			"]" << std::endl;

			// Run the network listener in a separate thread
			netThread = std::unique_ptr<std::thread>(new std::thread([&] {
				io_service.run();
			}));

			viewer.attachSocket(socket);
		}

		// Load annotations, if any, and run
		if (Settings::getInstance().ANNOTATIONS != "none")
			viewer.loadAnnotations(Settings::getInstance().ANNOTATIONS);

		viewer.display(mesh, renderer);
		
		// Stop networking and join to main thread
		if (Settings::getInstance().NETWORK_ENABLED) {
			io_service.stop();
			netThread->join();
		}

	} catch (std::exception &e) {
		std::cout << "Runtime error: "<< e.what() << std::endl;
		std::cin.get();
		return -1;
	}

	return 0;
}


void initShader(std::shared_ptr<GLShader> &shader) {
	shader->init(
		// Name
		"std-shader",

		// Vertex shader
		std::string("#version 330") + "\n" +

		"uniform mat4 mvp;" + "\n" +
		"uniform mat4 modelViewMatrix;" + "\n" +
		"uniform mat3 normalMatrix;" + "\n" +
		"uniform bool useSpecular = false;" + "\n" +

		"in vec3 position;" + "\n" +
		"in vec3 normal;" + "\n" +
		"in vec2 tex;" + "\n" +

		"out vec3 vertexNormal;" + "\n" +
		"out vec3 vertexPosition;" + "\n" +
		"out vec2 uv;" + "\n" +
		"out vec2 uvGI;" + "\n" +

		"void main () {" + "\n" +
		"    // Pass" + "\n" +
		"    uv = tex;" + "\n" +
		"    vertexNormal = normal;" + "\n" +
		"    vertexPosition = position;" + "\n" +

		"    // GI" + "\n" +
		"    vec3 e = normalize(vec3(modelViewMatrix * vec4(position, 1.0)));" + "\n" +
		"    vec3 n = normalize(normalMatrix * normal);" + "\n" +

		"    vec3 r;" + "\n" +
		"    if (useSpecular)" + "\n" +
		"		r = reflect(e, n);" + "\n" +
		"    else" + "\n" +
		"		r = n;" + "\n" +

		"    float m = 2.0 * sqrt(pow(r.x, 2.0) + pow(r.y, 2.0) + pow(r.z + 1.0, 2.0));" + "\n" +
		"    uvGI = r.xy / m + .5;" + "\n" +

		"    gl_Position = mvp * vec4(position, 1.0);" + "\n" +
		"}" + "\n",

		// Fragment shader
		std::string("#version 330") + "\n" +

		"// Point light representation" + "\n" +
		"struct Light {" + "\n" +
		"    vec3 position;" + "\n" +
		"    vec3 intensity;" + "\n" +
		"	 float ambientCoefficient;" + "\n" +
		"};" + "\n" +

		"uniform sampler2D env;" + "\n" +
		"uniform sampler2D envDiffuse;" + "\n" +
		"uniform Light light;" + "\n" +
		"uniform vec3 materialColor;" + "\n" +
		"uniform mat4 modelMatrix;" + "\n" +
		"uniform mat3 normalMatrix;" + "\n" +
		"uniform float alpha;" + "\n" +
		"uniform bool simpleColor = false;" + "\n" +
		"uniform bool textureOnly = false;" + "\n" +
		"uniform bool enableGI = false;" + "\n" +
		"uniform bool specular = false;" + "\n" +
		"uniform vec3 cameraPosition;" + "\n" +

		"in vec3 vertexNormal;" + "\n" +
		"in vec3 vertexPosition;" + "\n" +
		"in vec2 uv;" + "\n" +
		"in vec2 uvGI;" + "\n" +

		"out vec4 color;" + "\n" +

		"void main () {" + "\n" +
	
		"    // Shading" + "\n" +
		"    if (!simpleColor && !textureOnly) {" + "\n" +
	
		"        // Transform normal" + "\n" +
		"        vec3 normal = normalize(normalMatrix * vertexNormal);" + "\n" +

		"        // Position of fragment in world coodinates" + "\n" +
		"        vec3 position = vec3(modelMatrix * vec4(vertexPosition, 1.0));" + "\n" +

		"        // Calculate the vector from surface to the light" + "\n" +
		"        vec3 surfaceToLight = light.position - position;" + "\n" +
		"        vec3 surfaceToCamera = light.position - cameraPosition;" + "\n" +

		"        // Calculate the cosine of the angle of incidence = brightness" + "\n" +
		"        float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));" + "\n" +
		"        brightness = clamp(brightness, 0.0, 1.0);" + "\n" +

		"		 // Lightning" + "\n" +
		"		 if (enableGI) {" + "\n" +
		"			vec3 gi = (specular ? texture(env, uvGI).rgb : texture(envDiffuse, uvGI).rgb);" + "\n" +

		"			// Ambient" + "\n" +
		"			vec3 ambient = vec3(light.ambientCoefficient * materialColor * light.intensity);" + "\n" +

		"			// Diffuse" + "\n" +
		"			vec3 diffuse = vec3(gi * brightness * light.intensity);" + "\n" +

		"			// Specular" + "\n" +
		"			float specularCoefficient = 0.0;" + "\n" +
		"			float materialShininess = 1.5;" + "\n" +
		"			if (brightness > 0.0)" + "\n" +
		"				specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), materialShininess);" + "\n" +
		
		"			vec3 specularC = specularCoefficient * materialColor * light.intensity;" + "\n" +

		"			float k = 0.2;" + "\n" +
		"			float attenuation = 1.0 / (1.0 + k * pow(length(surfaceToLight), 2));" + "\n" +

		"			color = vec4(ambient + attenuation * (diffuse + specularC), alpha);" + "\n" +

		"		 } else {" + "\n" +
		"			color = vec4(materialColor * brightness * light.intensity, alpha);" + "\n" +
		"		 }" + "\n" +
		"    } else if (textureOnly) {" + "\n" +
		"        // No shading, only textures" + "\n" +
		"        color = texture(env, uv.xy);" + "\n" +
		"    } else {" + "\n" +
		"        // Draw all in simple colors" + "\n" +
		"        color = vec4(materialColor, alpha);" + "\n" +
		"    }" + "\n" +
		"}" + "\n"
	);
}
