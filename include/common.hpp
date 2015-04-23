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
	// Prevent OpenGL Compiler warnings
	# define __gl_h_
	# define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#elif __linux
	#define PLATFORM_LINUX
#endif

// Expose Rift to GLFW
#if defined(PLATFORM_WINDOWS)
 #define GLFW_EXPOSE_NATIVE_WIN32
 #define GLFW_EXPOSE_NATIVE_WGL
 #define OVR_OS_WIN32
#elif defined(PLATFORM_APPLE)
 #define GLFW_EXPOSE_NATIVE_COCOA
 #define GLFW_EXPOSE_NATIVE_NSGL
 #define OVR_OS_MAC
#elif defined(PLATFORM_LINUX)
 #define GLFW_EXPOSE_NATIVE_X11
 #define GLFW_EXPOSE_NATIVE_GLX
 #define OVR_OS_LINUX
#endif

// Include the basis usages
#include "Eigen/Core"
#include <stdint.h>
#include <iostream>
#include <vector>
#if defined(PLATFORM_APPLE)
	#define GLFW_INCLUDE_GLCOREARB
#elif defined(PLATFORM_WINDOWS)
	#define NOMINMAX
	#include <windows.h>
    #define GLEW_STATIC
    #include <GL/glew.h>
	#define _USE_MATH_DEFINES
#else
    #define GL_GLEXT_PROTOTYPES
#endif
#include <memory>
#include <math.h>
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
#include "OVR_CAPI.h"
#include "OVR_Math.h"
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
typedef Eigen::Matrix<float, 3, 1> Color3f;
typedef Eigen::Matrix<float, 3, 1> Normal3f;
typedef Eigen::Matrix<unsigned int, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> MatrixXf;
typedef Eigen::Matrix<unsigned int, 3, 1> Vector3ui;

typedef OVR::Matrix4f OMatrix4f;
typedef OVR::Matrix3f OMatrix3f;
typedef OVR::Vector3f OVector3f;
typedef OVR::Quatf OQuatf;

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

/// Indent a string by the specified number of spaces
extern std::string indent (const std::string &string, int amount = 2);

VR_NAMESPACE_END
