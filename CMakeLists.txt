# Basis skeleton for this build file taken from Nori/Nanogui project created by Jakob Wenzel

cmake_minimum_required (VERSION 2.8.3)
project("VRMeshViewer")

# Args for nested build process
set(CMAKE_EXTERNAL_ARGUMENTS_LIST "")
include(ExternalProject)

# =========================================
# External dependencies
# =========================================

# GLFW
ExternalProject_Add(glfw_p
	URL "${CMAKE_SOURCE_DIR}/ext/glfw"
	PREFIX "ext_build"
	CMAKE_ARGS ${CMAKE_EXTERNAL_ARGUMENTS_LIST} -DGLFW_BUILD_TESTS=0 -DGLFW_BUILD_EXAMPLES=0 -DGLFW_BUILD_DOCS=0 -DGLFW_INSTALL=0
	INSTALL_COMMAND ""
)

# Rift SDK
# Create debug symbols
set(OVR_DEBUG_BUILD 1)

# The primary SDK artifact, a static library for Oculus access 
add_subdirectory(ext/ovr)
set(extra_libs ovr)

if (WIN32)
	# Windows needs GLEW
	ExternalProject_Add(glew_p
		URL "${CMAKE_SOURCE_DIR}/ext/glew"
		PREFIX "ext_build"
		CMAKE_ARGS ${CMAKE_EXTERNAL_ARGUMENTS_LIST}
		INSTALL_COMMAND ""
	)
	
	# No akward security warnings
	add_definitions (/D "_CRT_SECURE_NO_WARNINGS")
	
	include_directories(ext/glew/include)
	set(extra_libs ${extra_libs} opengl32
		$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/ext_build/src/glew_p-build/Debug/glew.lib>
		$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/ext_build/src/glew_p-build/Release/glew.lib>
		$<$<CONFIG:RelWithDebInfo>:${CMAKE_BINARY_DIR}/ext_build/src/glew_p-build/RelWithDebInfo/glew.lib>
		$<$<CONFIG:MinSizeRel>:${CMAKE_BINARY_DIR}/ext_build/src/glew_p-build/MinSizeRel/glew.lib>
	)
elseif (APPLE)
	set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
	
	# Find standard libraries needed for the Rift and GLFW
	find_library(cocoa_library Cocoa)
	find_library(opengl_library OpenGL)
	find_library(corevideo_library CoreVideo)
	find_library(core_foundation CoreFoundation)
    find_library(core_graphics CoreGraphics)
	find_library(iokit_library IOKit)
	
	set(extra_libs ${extra_libs} ${cocoa_library} ${opengl_library} ${corevideo_library} ${iokit_library} ${core_foundation} ${core_graphics})
elseif ("${CMAKE_SYSTEM}" MATCHES "Linux")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(extra_libs ${extra_libs} GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 pthread dl)
endif()

# Set include directories
include_directories(
	ext/eigen
	ext/glfw/include
	ext/tinyformat
	ext/filesystem
	ext/stb_image
	ext/ovr/LibOVR/include
	include
	${CMAKE_BINARY_DIR}
)

# The following lines build the main executable
add_executable(VRMeshViewer

	# Header files
	include/common.hpp
	include/Viewer.hpp
	include/GLUtil.hpp
	include/mesh/Mesh.hpp
	include/mesh/ObjectLoader.hpp
	include/renderer/Renderer.hpp
	include/renderer/PerspectiveRenderer.hpp
	include/renderer/RiftRenderer.hpp
#	include/mesh/Texture.hpp
	
	# Source code files
	src/main.cpp
	src/common.cpp
	src/GLUtil.cpp
	src/Viewer.cpp
	src/mesh/Mesh.cpp
	src/mesh/ObjectLoader.cpp
	src/renderer/PerspectiveRenderer.cpp
	src/renderer/RiftRenderer.cpp
#	src/mesh/Texture.cpp
)

# Make sure to build in the right order
add_dependencies(VRMeshViewer glfw_p)
if(WIN32)
  add_dependencies(VRMeshViewer glew_p)
endif()

# Link to dependent libraries
link_directories(${CMAKE_BINARY_DIR}/ext_build/src/glfw_p-build/src)
target_link_libraries(VRMeshViewer glfw3 ${extra_libs})

# Copy resources
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
