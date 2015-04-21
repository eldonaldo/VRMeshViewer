#include "renderer/RiftRenderer.hpp"

VR_NAMESPACE_BEGIN

RiftRenderer::RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: PerspectiveRenderer(shader, fov, width, height, zNear, zFar) {

	// Reset all
	setViewMatrix(Matrix4f::Identity());
	setProjectionMatrix(Matrix4f::Identity());
	setModelMatrix(Matrix4f::Identity());

}

RiftRenderer::~RiftRenderer() {

}

void RiftRenderer::preProcess () {
	PerspectiveRenderer::preProcess();

	if (hmd == nullptr)
		throw new VRException("HMD not set! Can't do pre processing for the Rift");

	// Configure Stereo settings.
	ovrFovPort eyeFov[2] = { hmd->DefaultEyeFov[0], hmd->DefaultEyeFov[1] };
	OVR::Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, eyeFov[0], 1.0f);
	OVR::Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, eyeFov[1], 1.0f);

	// Calculate texture dimension needed
	renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
	renderTargetSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);

	// Generate framebuffer for rendering
	int multiSample = 1;
	frameBuffer.init(Vector2i(renderTargetSize.w, renderTargetSize.h), multiSample, true);
	frameBuffer.release();

	// Set drawing spaces for the left and right eyes
	eyeRenderViewport[0].Pos = OVR::Vector2i(0, 0);
	eyeRenderViewport[0].Size = OVR::Sizei(renderTargetSize.w / 2, renderTargetSize.h);
	eyeRenderViewport[1].Pos = OVR::Vector2i((renderTargetSize.w + 1) / 2, 0);
	eyeRenderViewport[1].Size = eyeRenderViewport[0].Size;

	// Rendering texture for the left eye
	eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	eyeTexture[0].OGL.Header.TextureSize = renderTargetSize;
	eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
	eyeTexture[0].OGL.TexId = frameBuffer.getColor();

	// Rendering texture for the right eye
	eyeTexture[1] = eyeTexture[0];
	eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];

	// Configure the Rift to use OpenGL
	cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	cfg.OGL.Header.BackBufferSize = OVR::Sizei(FBWidth, FBHeight);
	cfg.OGL.Header.Multisample = 0;

// Need to attach window for direct rendering on windows (only supported in windows)
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

void RiftRenderer::update () {
	PerspectiveRenderer::update();

	// Query the HMD for the current tracking state.
//	ovrTrackingState ts  = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
//	if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)) {
//		ovrPoseStatef pose = ts.HeadPose;
//	}
}

void RiftRenderer::clear (Vector3f background) {
	frameBuffer.bind();
	PerspectiveRenderer::clear(background);
}

void RiftRenderer::draw () {
//	static float yaw(1);
//	static OVector3f pos2(0, 0, 0);
//	pos2.y = ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, pos2.y);

	ovrFrameTiming frameTiming = ovrHmd_BeginFrame(hmd, 0);

	// Get eye poses, feeding in correct IPD offset
	ovrVector3f viewOffset[2] = {eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset};
	ovrPosef eyeRenderPose[2];
	ovrHmd_GetEyePoses(hmd, 0, viewOffset, eyeRenderPose, NULL);

	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];

		// Get view and projection matrices
//		OMatrix4f rollPitchYaw       = OMatrix4f::RotationY(yaw);
//		OMatrix4f finalRollPitchYaw  = rollPitchYaw * OMatrix4f(eyeRenderPose[eye].Orientation);
//		OVector3f finalUp            = finalRollPitchYaw.Transform(OVector3f(0, 1, 0));
//		OVector3f finalForward       = finalRollPitchYaw.Transform(OVector3f(0, 0, -1));
//		OVector3f shiftedEyePos      = pos2 + rollPitchYaw.Transform(eyeRenderPose[eye].Position);

//		OMatrix4f view = OMatrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
//		OMatrix4f view = OMatrix4f::LookAtRH(
//			OVector3f(4, 3, -3),
//			OVector3f(0, 0, 0),
//			OVector3f(0, 1, 0)
//		);
//
//		OMatrix4f proj = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_RightHanded);
//
//		Matrix4f v = Eigen::Map<Matrix4f>((float *) view.M);
//		Matrix4f p = Eigen::Map<Matrix4f>((float *) proj.M);
//		Vector3f pos(shiftedEyePos.x, shiftedEyePos.y, shiftedEyePos.z);

//		setViewMatrix(v);
//		setProjectionMatrix(p);

		setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));

		setViewMatrix(lookAt(
			Vector3f(0, 0, 5), // Camera is at (0,0,10), in world space
			Vector3f(0, 0, 0), // And looks at the origin
			Vector3f(0, 1, 0) // Head is up
		));

		shader->bind();
		shader->setUniform("light.position", Vector3f(0, 0, 5));
		shader->setUniform("modelMatrix", getModelMatrix());
		shader->setUniform("normalMatrix", getNormalMatrix());
		shader->setUniform("mvp", getMvp());

		glViewport(eyeRenderViewport[eye].Pos.x, eyeRenderViewport[eye].Pos.y, eyeRenderViewport[eye].Size.w, eyeRenderViewport[eye].Size.h);

		// Draw the mesh for each eye
		shader->drawIndexed(GL_TRIANGLES, 0, mesh->getNumFaces());
	}

	frameBuffer.blit();
	ovrHmd_EndFrame(hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
