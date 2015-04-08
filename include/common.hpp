#pragma once

// Determine on which OS we are operating on ...
#define PATH_SEPARATOR "/"
#ifdef _WIN32
	#define PLATFORM_WINDOWS
	#undef PATH_SEPARATOR
	#define PATH_SEPARATOR "\\"
	#pragma warning(disable : 4244)
#elif __APPLE__
	#define PLATFORM_APPLE
#elif __linux
	#define PLATFORM_LINUX
#endif

// Include the basis usages
#include "Eigen/Core"
#include <stdint.h>
#include <array>
#include <vector>
#if defined(PLATFORM_APPLE)
	#define GLFW_INCLUDE_GLCOREARB
#elif defined(PLATFORM_WINDOWS)
	#include <windows.h>
    #define GLEW_STATIC
    #include <GL/glew.h>
#else
    #define GL_GLEXT_PROTOTYPES
#endif
#include "GLFW/glfw3.h"
#include "tinyformat.h"

// Convenience definitions
#define VR_NS vrmv
#define VR_NAMESPACE_BEGIN namespace VR_NS {
#define VR_NAMESPACE_END }

namespace VR_NS {}
using namespace vrmv;

// Some imports and common typedefs
using Eigen::Vector2f;
using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::Vector2i;
using Eigen::Vector3i;
using Eigen::Vector4i;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::MatrixXf;

VR_NAMESPACE_BEGIN

// Some typedefs
typedef Vector3f Color3f;

/// Stores an RGBA color value
class Color : public Eigen::Vector4f {
public:

	Color() { }

	Color(int intensity, int alpha) {
		const float scale = 1.f / 255.f;
		x() = y() = z() = intensity * scale; w() = alpha * scale;
	}

	Color(int r, int g, int b, int a) {
		const float scale = 1.f / 255.f;
		x() = r * scale; y() = g * scale;
		z() = b * scale; w() = a * scale;
	}
};

// Simple exception class, which stores a human-readable error description
class VRException : public std::runtime_error {
public:
    // Variadic template constructor to support printf-style arguments
    template <typename... Args>
    VRException (const char *fmt, const Args &... args)
		: std::runtime_error(tfm::format(fmt, args...)) {

    }
};

/// Copy of non-existing std::to_string for MinGW
template <typename T>
std::string toString (T value) {
	std::ostringstream os;
	os << value;
	return os.str();
}

/// cot(x) = tan(Pi - x)
inline float cot (float x) {
	return tan(M_PI - x);
}

/// Degrees to radiant
inline float degToRad (float deg) {
	return deg * float(M_PI / 180.0);
}

/// Radiant to degrees
inline float radToDeg (float rad) {
	return rad * float(180.0 / M_PI);
}

VR_NAMESPACE_END
