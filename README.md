Gesture-driven Virtual Reality Mesh Viewer
==========================================

VRMeshViewer is a virtual reality mesh viewer, that allows the user to inspect 3D models using the Oculus Rift (at least DK2), 
and transform the model’s state by forming gestures using the tracking data provided by the Leap Motion mounted in front of the Rift 
using the Oculus Rift Development Mount. 

It features two different operating modes: augmented reality and spherical virtual environment. Whereas in augmented reality mode 
the Leap camera images are used as background stream, turning the application into a augmented reality mesh viewer where the virtual
hands are synchronized with the real (or even disable them completely by pressing `h`), the spherical virtual environment renders
a virtual living room where the model is shaded according to the light distribution of the environment map.

The mesh viewer was built during my undergrate thesis and is a reserch project, meaning that it may contain bugs or even may not work 
on your system (see section Description).

## Available Gestures

### Scaling and Translation

![Scaling and Translation Gesture](https://github.com/nicoprevitali/VRMeshViewer/raw/master/figures/gesture-scaling.png "Scaling and Translation Gesture")

To start the gesture, fully extend all fingers, put both hands on the same height as the model and align them such that the palm of the left and right hand point towards the object. 
If this alignment is held for roughly a half second, the application recognizes it as an attempt to start 
the gesture. Unfortunately, the Leap Motion software is not able to track 
bent fingers in a way that is as robust as tracking of extended fingers. For this reason, the user needs 
to extend his fingers instead of making a fist when simulating the grasp.

### Rotation

![Rotation Gesture](https://github.com/nicoprevitali/VRMeshViewer/raw/master/figures/gesture-rotation.png "Rotation Gesture")

To start the gesture, fully extend all fingers (whereat the thumb may be relaxed) and place the hand in space, where you would expect 
the surface of bounding sphere to be, keeping it steady for a small amount of time. If you were wrong with your guess, the sphere’s mesh 
starts to blink in red, pointing you to the correct position. In case the you estimated correctly, the gesture starts. 
During the rotation the spheres’s mesh is displayed, acting as a visual hint, so that you know where to move your hand in order to match and 
follow the sphere’s surface when rotating. To stop the gesture, the you can either make a fist, relax some fingers, or move the hand away from 
the model whereat a certain distance the gesture stops.

### Annotation Placement

![Annotation Placement Gesture](https://github.com/nicoprevitali/VRMeshViewer/raw/master/figures/gesture-annotation.png "Annotation Placement Gesture")

Annotations are little pins that stick perpendicular to the model's surface, placed by your hands through executing a gesture. 
To place an annotation, pinch the thumb and index finger together and extend all other finders. Then move the hand towards to model until you 
hit the surface with your pinched fingers. The annotation is then placed, where the your touched the surface. 
The gesture corresponds to placing a pin onto a pin board. To place another pin your must repeat the pinch. 
By forming the same gesture but touching an already placed pin, you can remove it.

## Keymap

| Key  |Description |
| ------------- | ------------- |
|Esc | Quit application|
|A | Save annotations to a file|
|B | Draw bounding box|
|C | Reset model state|
|E | Show / hide pedestal for the model|
|G | Enable / disable virtual environment|
|H | Show / hide virtual hands|
|M | Show / hide model|
|P | Enable / disable passthrough|
|R | Recenter world coordinate system origin to current head position|
|S | Show / hide rotation sphere|
|V | Enable / disable v-sync|
|W | Draw wireframe|

## Dependencies

VRMeshViewer builds on [GLFW](http://www.glfw.org/) for cross-platform OpenGL context creation and event handling, 
[GLEW](http://glew.sourceforge.net/) to use OpenGL 3.x on Windows, 
[Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page) for basic vector types and certainly it depends on the
Oculus Rift and Leap Motion runtime which are not included in the repo.

VRMeshViewer currently works on Mac OS X (Clang) and Windows
(Visual Studio ≥ 2013); it requires a recent C++11 capable compiler. All
dependencies are jointly built using a CMake-based build system.

## Runtime Requirements

#### Windows

* At least Oculus Rift Runtime 0.5.0 installed
* At least Leap SDK 2.2.5 installed  and environment variable "LEAPSDK_DIR" set, pointing to the folder where the SDK resides.

#### Mac OS X

* At least Oculus Rift Runtime 0.5.0 installed
* At least Leap SDK / Runtime 2.2.5 installed

## Compiling

Clone the repository and all dependencies (with `git clone --recursive`),
run CMake to generate Makefiles or CMake/Visual Studio project files, and
the rest should just work automatically.