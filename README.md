Gesture-driven Virtual Reality Mesh Viewer
==========================================

Bachelor Thesis

## Requirements

#### Windows

* Oculus Rift Runtime 0.5.0 installed
* LeapSDK/Runtime 2.2.5 installed and environment variable "LEAPSDK_DIR" set, pointing to the folder where the SDK resides

#### Mac

* Oculus Rift Runtime 0.5.0 installed
* LeapSDK/Runtime 2.2.5 installed

#### Leap Motion

* Disable "Robust Mode Tracking" in the Leap Motion settings. Currently there is no way to query if the device
is in robust mode. If the device is in robust mode, it frequently happens that the Leap images sizes change and thus
the OpenGL buffers are not correct anymore.

## Installation

1. clone repo
2. cd path/to/repo/../
3. mkdir build; cd build
4. cmake ../repo
5. make