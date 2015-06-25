#pragma once

#include "common.hpp"
#include "nanogui/screen.h"
#include "Viewer.hpp"

VR_NAMESPACE_BEGIN

/**
 * @brief Gui for the mesh viewer. Only applicable in 2D mode.
 */
class Gui : public nanogui::Screen, public Viewer {

public:

	Gui();
	virtual ~Gui();
	void drawContents();

	virtual bool mouseButtonEvent (const Vector2i &p, int button, bool down, int modifiers) override;

	virtual bool mouseMotionEvent (const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;

	virtual bool scrollEvent (const Vector2i &p, const Vector2f &rel) override;

	virtual bool mouseDragEvent (const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;

	virtual bool mouseEnterEvent (const Vector2i &p, bool enter) override;

	virtual void framebufferSizeChanged () override;

	bool keyboardEvent(int key, int scancode, bool action, int mods);
};

VR_NAMESPACE_END
