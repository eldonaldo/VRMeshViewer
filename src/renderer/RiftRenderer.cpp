#include "renderer/RiftRenderer.hpp"

VR_NAMESPACE_BEGIN

RiftRenderer::RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: PerspectiveRenderer(shader, fov, width, height, zNear, zFar) {

	hmd = ovrHmd_Create(0);
	if (!hmd)
		hmd = ovrHmd_CreateDebug(ovrHmdType::ovrHmd_DK2);
		if (!hmd)
			throw VRException("Could not start the Rift");

	// Initialize and enable sensors
	if (!ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0))
		throw VRException("Rift does not support all of the necessary sensors");
}

RiftRenderer::~RiftRenderer() {
	// Destroy the rift. Needs to be called after glfwTerminate
	ovrHmd_Destroy(hmd);
	ovr_Shutdown();
}

void RiftRenderer::preProcess () {
	PerspectiveRenderer::preProcess();

	ovrFovPort eyeFov[2] = {
		hmd->DefaultEyeFov[0],
		hmd->DefaultEyeFov[1]
	};

	// Configure Stereo settings.
	OVR::Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, eyeFov[0], 1.0f);
	OVR::Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, eyeFov[1], 1.0f);

	// Calculate texture dimension needed
	renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
	renderTargetSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);

	// Generate Framebuffer for rendering
	int multiSample = 1;
	frameBuffer.init(Vector2i(renderTargetSize.w, renderTargetSize.h), multiSample, true);
	frameBuffer.release();

	// Set drawing spaces for the left and right eyes
	eyeRenderViewport[0].Pos = OVR::Vector2i(0, 0);
	eyeRenderViewport[0].Size = OVR::Sizei(renderTargetSize.w / 2, renderTargetSize.h);
	eyeRenderViewport[1].Pos = OVR::Vector2i((renderTargetSize.w + 1) / 2, 0);
	eyeRenderViewport[1].Size = eyeRenderViewport[0].Size;

	// Texture for the left eye
	eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	eyeTexture[0].OGL.Header.TextureSize = renderTargetSize;
	eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
	eyeTexture[0].OGL.TexId = frameBuffer.getColor();

	// Texture for the right eye
	eyeTexture[1] = eyeTexture[0];
	eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];

	// Configure the Rift to use OpenGL
	cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	cfg.OGL.Header.BackBufferSize = OVR::Sizei(FBWidth, FBHeight);
	cfg.OGL.Header.Multisample = multiSample;
#if defined(PLATFORM_WINDOWS)
	cfg.OGL.Window = glfwGetWin32Window(window);
	if (!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
		ovrHmd_AttachToWindow(hmd, window, NULL, NULL);
#endif

	// We use distortion rendering
	ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive, eyeFov, eyeRenderDesc);
	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
}

//GLFWMonitor *RiftRenderer::getHmdDisplay () {
//int i, count;
//GLFWmonitor** monitors = glfwGetMonitors(&count);
//for (i = 0;  i < count;  i++)
//{
//#if defined(_WIN32)
//if (strcmp(glfwGetWin32Monitor(monitors[i]), hmd->DisplayDeviceName) == 0)
//return monitors[i];
//#elif defined(__APPLE__)
//if (glfwGetCocoaMonitor(monitors[i]) == hmd->DisplayId)
//return monitors[i];
//#elif defined(__linux__)
//int xpos, ypos;
//const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
//glfwGetMonitorPos(monitors[i], &xpos, &ypos);
//if (hmd->WindowsPos.x == xpos &&
//hmd->WindowsPos.y == ypos &&
//hmd->Resolution.w == mode->width &&
//hmd->Resolution.h == mode->height)
//{
//return monitors[i];
//}
//#endif
//}
//}

void RiftRenderer::update () {
	PerspectiveRenderer::update();

	// Query the HMD for the current tracking state.
	ovrTrackingState ts  = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) {
		ovrPoseStatef pose = ts.HeadPose;
	}
}

void RiftRenderer::clear (Vector3f background) {
	frameBuffer.bind();
	PerspectiveRenderer::clear(background);
}

void RiftRenderer::draw () {
	ovrFrameTiming frameTiming = ovrHmd_BeginFrame(hmd, 0);

//	PerspectiveRenderer::draw();

	ovrPosef eyeRenderPose[2];
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];
		eyeRenderPose[eye] = ovrHmd_GetHmdPosePerEye(hmd, eye);

		OVR::Matrix4f MVPMatrix =
			OVR::Matrix4f(ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, zNear, zFar, true)) *
			OVR::Matrix4f::Translation(eyeRenderDesc[eye].HmdToEyeViewOffset) *
			OVR::Matrix4f(OVR::Quatf(eyeRenderPose[eye].Orientation).Inverted()) *
			OVR::Matrix4f::Translation(-OVR::Vector3f(eyeRenderPose[eye].Position));

//		mvp << MVPMatrix;
		shader->bind();
		shader->setUniform("modelMatrix", getModelMatrix());
		shader->setUniform("normalMatrix", getNormalMatrix());
		glUniformMatrix4fv(shader->uniform("mvp"), 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

		// Set rendering area on renderbuffer for the current eye
		glViewport(eyeRenderViewport[eye].Pos.x, eyeRenderViewport[eye].Pos.y, eyeRenderViewport[eye].Size.w, eyeRenderViewport[eye].Size.h);

		shader->drawIndexed(GL_TRIANGLES, 0, mesh->getNumFaces());
	}

	frameBuffer.blit();
	ovrHmd_EndFrame(hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
