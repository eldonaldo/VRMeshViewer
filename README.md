Gesture-driven Virtual Reality Mesh Viewer
==========================================

VRMeshViewer is a virtual reality tool, that allows the user to inspect 3D models in OBJ wavefront format using the Oculus Rift DK2 
and transform the model by forming simple gestures using a Leap Motion mounted in front of the Oculus Rift.

It features two different operating modes: augmented reality and virtual environment. In augmented reality mode, 
the Leap camera images are used as background stream, turning the application into a augmented reality mesh viewer where the virtual
hands are synchronized with the real (or you can even disable them completely by pressing `h`). In virtual environment, the application renders
a virtual living room where the model is shaded according to the light distribution of the environment map. By interconnecting two instances 
through network communication, the viewer allows two users to synchronize the state of the application, while maintaining different viewpoints. 
Annotations are then displayed identically in both instances, making it more clear which part of the model is being discussed.

The mesh viewer was built during my undergrate thesis and is a reserch project in the field of VR devices. This means, that it may 
contain bugs or even may not work on your platform (see section Description).

## Screenshots

### Scaling Gesture

![Scaling and Translation Gesture](https://github.com/nicoprevitali/VRMeshViewer/raw/master/figures/vrmv-scaling.png "Scaling and Translation Gesture")

### Augmented Reality

![Augmented Reality Scaling and Translation Gesture](https://github.com/nicoprevitali/VRMeshViewer/raw/master/figures/vrmv-augmented-reality.png "Augmented Reality Scaling and Translation Gesture")

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

## Usage

* _Mandatory first argument_. Besides the virtual reality mode the application features a regular 2D perspective, and therefore the first program argument specifies in which mode we want to start the application, i.e. `2d` or `3d`.

* _Mandatory second argument_. The second arugment needs to provide the path to the model wavefront OBJ geometry file, i.e. `path-to-model.obj`

* _Optional thrid argument_. Specifies, if any previously saved annotations should be loaded. If this is the case, provide the path to the file. If not, provide `none`

* _Optional fourth argument_. Specifies the networking mode used if any. I.e. the leader passes `server`, and the viewer `client`. The next two arguments are the other's instance `UDP port` and `IP address`

Hence, call

`VRMeshViewer[.exe] <3d|2d> <model.obj> [<none|annotations.txt>] [<client|server> <UDP-port> <ip-address>]`

## Keymap

| Key  |Function |
| ------------- | ------------- |
|Esc | Quit application|
|A | Save annotations to a file in the current working directory|
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

* At least Oculus Rift Runtime 0.5.0 installed
* At least Leap SDK / Runtime 2.2.5 installed

## Compiling

First, set an user and system environment variable called `LEAPSDK_DIR`, 
pointing to the folder where the Leap Motion SDK resides.

Then, clone the repository and all dependencies (with `git clone --recursive`),
run CMake to generate Makefiles or CMake/Visual Studio project files, and
the rest should just work automatically.