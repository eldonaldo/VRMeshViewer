#include "Gui.hpp"
#include "nanogui/layout.h"
#include "nanogui/window.h"

VR_NAMESPACE_BEGIN

using namespace nanogui;

Gui::Gui ()
	: Screen(Vector2i(Settings::getInstance().WINDOW_WIDTH, Settings::getInstance().WINDOW_HEIGHT)
	, Settings::getInstance().TITLE, true, Settings::getInstance().FULLSCREEN),
	Viewer(Settings::getInstance().TITLE, Settings::getInstance().WINDOW_WIDTH, Settings::getInstance().WINDOW_HEIGHT
	, Settings::getInstance().FULLSCREEN) {


	// Set pointer to GLFW viewerGLFWwindow
	viewerGLFWwindow = glfwWindow();

	Window *windw = new Window(this, "Button demo");
	windw->setPosition(Vector2i(15, 15));
	windw->setLayout(new GroupLayout());

	performLayout(mNVGContext);

	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (!Settings::getInstance().USE_RIFT)
		glEnable(GL_MULTISAMPLE);

	drawAll();
	setVisible(true);
}

void Gui::drawContents () {

	// Last send time in milliseconds
	static long lastTime = glfwGetTime() * 1000;

	// Draw contents
	if (ready)
		render(lastTime);
}

bool Gui::keyboardEvent(int key, int scancode, bool action, int mods) {
	switch (key) {
		// Recenter HDM pose
		case GLFW_KEY_R:
			if (action == GLFW_PRESS)
				ovrHmd_RecenterPose(hmd);
			break;

		// Enable / disable v-sync
		case GLFW_KEY_V: {
			static bool disable = true;
			if (action == GLFW_PRESS && Settings::getInstance().USE_RIFT) {
				if (disable)
					ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction | ovrHmdCap_NoVSync);
				else
					ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
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
			getTranslateMatrix() = Matrix4f::Identity();
			getScaleMatrix() = Matrix4f::Identity();
			getTranslateMatrix() = Matrix4f::Identity();
			break;
		}

		// Save annotations to a file
		case GLFW_KEY_A: {
			if (action == GLFW_PRESS)
				saveAnnotations();

			break;
		}

		// Enable/disable leap for 2d use
		case GLFW_KEY_L: {
			static bool disable = true;
			if (action == GLFW_PRESS) {
				Settings::getInstance().USE_LEAP = disable && leapController.isConnected();
				disable = !disable;
			}

			break;
		}
	}

	Screen::keyboardEvent(key, scancode, action, mods);
}

void Gui::framebufferSizeChanged () {
	Screen::framebufferSizeChanged();

	FBWidth = mFBSize.x();
	FBHeight = mFBSize.y();

	// Get the window size, not framebuffer size
	int Wwidth, Wheight;
	glfwGetWindowSize(viewerGLFWwindow, &Wwidth, &Wheight);
	glViewport(0, 0, Wwidth, Wheight);
	getArcball().setSize(Vector2i(Wwidth, Wheight));

	// Propagate
	Viewer::width = Wwidth;
	Viewer::height = Wheight;

	if (renderer)
		renderer->updateFBSize(FBWidth, FBHeight);
}

bool Gui::mouseButtonEvent (const Vector2i &p, int button, bool down, int modifiers) {
	if (!Settings::getInstance().USE_RIFT && glfwGetKey(viewerGLFWwindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && down) {
		// Query viewport
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		Vector2i viewPortSize(viewport[2], viewport[3]);

		// Query cursor position and depth value at this position
		double x, y; GLfloat z;
		glfwGetCursorPos(viewerGLFWwindow, &x, &y);
		glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		Vector3f pos(x, viewport[3] - y, z);

		// Unproject scene
		Matrix4f VM = getRenderer()->getViewMatrix() * getMesh()->getModelMatrix();
		Vector3f worldPosition = unproject(pos, VM, getRenderer()->getProjectionMatrix(), viewPortSize);

		// Add/Delete an annotation
		Vector3f n(0.f, 1.f, 0.f);
		if (!deletePinIfHit(worldPosition))
			addAnnotation(worldPosition, n);

//		cout << z << endl;
//		ppv(worldPosition);
	} else if (button == GLFW_MOUSE_BUTTON_LEFT) {
		arcball.button(lastPos, down);
	}

	// Need to send a new packet
	Settings::getInstance().NETWORK_NEW_DATA = true;

	return Widget::mouseButtonEvent(p, button, down, modifiers);
}

bool Gui::mouseMotionEvent (const Vector2i &p, const Vector2i &rel, int button, int modifiers) {
	lastPos = Vector2i(int(p.x()), int(p.y()));
	arcball.motion(lastPos);

	// Need to send a new packet
	if (arcball.active())
		Settings::getInstance().NETWORK_NEW_DATA = true;

	return Widget::mouseMotionEvent(p, rel, button, modifiers);
}

bool Gui::scrollEvent (const Vector2i &p, const Vector2f &rel) {
	if (Settings::getInstance().NETWORK_ENABLED && Settings::getInstance().NETWORK_MODE == NETWORK_MODES::CLIENT)
		return false;

	float scaleFactor = 0.05f;
#if defined(PLATFORM_WINDOWS)
	scaleFactor = 0.45f;
#endif

	if (rel.y() >= 0)
		scaleMatrix = scale(scaleMatrix, 1.f + scaleFactor);
	else
		scaleMatrix = scale(scaleMatrix, 1.f - scaleFactor);

	// Need to send a new packet
	Settings::getInstance().NETWORK_NEW_DATA = true;

	return Widget::scrollEvent(p, rel);
}

bool Gui::mouseDragEvent (const Vector2i &p, const Vector2i &rel, int button, int modifiers) {
	return Widget::mouseDragEvent(p, rel, button, modifiers);
}

bool Gui::mouseEnterEvent (const Vector2i &p, bool enter) {
	return Widget::mouseEnterEvent(p, enter);
}

Gui::~Gui () {
	// Renderer cleapup
	renderer->cleanUp();
}

VR_NAMESPACE_END
