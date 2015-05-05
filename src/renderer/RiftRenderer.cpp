#include "renderer/RiftRenderer.hpp"

VR_NAMESPACE_BEGIN

RiftRenderer::RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: yaw(0.f), PerspectiveRenderer(shader, fov, width, height, zNear, zFar) {

	// Reset all
	setViewMatrix(Matrix4f::Identity());
	setProjectionMatrix(Matrix4f::Identity());
}

RiftRenderer::~RiftRenderer() {

}

void RiftRenderer::preProcess () {
	// Upload mesh and set uniforms in shader
	PerspectiveRenderer::preProcess();

	if (hmd == nullptr)
		throw new VRException("HMD not set! Can't do pre necessary processing for the Rift");

	// Generate framebuffers for left and right eye
	OVR::Sizei texLeft = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[ovrEye_Left], 1.0f);
	frameBuffer[ovrEye_Left].init(Vector2i(texLeft.w, texLeft.h), 0, true);
	frameBuffer[ovrEye_Left].release();

	OVR::Sizei texRight = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[ovrEye_Right], 1.0f);
	frameBuffer[ovrEye_Right].init(Vector2i(texRight.w, texRight.h), 0, true);
	frameBuffer[ovrEye_Right].release();

	// Configure the Rift to use OpenGL
	cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	cfg.OGL.Header.BackBufferSize = hmd->Resolution;
	cfg.OGL.Header.Multisample = 0;
#if defined(PLATFORM_WINDOWS)
	// Need to attach window for direct rendering (only supported on windows)
	cfg.OGL.Window = glfwGetWin32Window(window);
	cfg.OGL.DC = GetDC(cfg.OGL.Window);
#endif

	// We use SDK distortion rendering
	ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive, hmd->DefaultEyeFov, eyeRenderDesc);
	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);

	// Dismiss warning
	ovrHmd_DismissHSWDisplay(hmd);
}

void RiftRenderer::update (Matrix4f &s, Matrix4f &r, Matrix4f &t) {
	PerspectiveRenderer::update(s, r, t);
}

void RiftRenderer::clear (Vector3f background) {
	frameBuffer[0].clear();
	frameBuffer[1].clear();
}

void RiftRenderer::draw () {
	// Begin distortion rendering
	ovrFrameTiming frameTiming = ovrHmd_BeginFrame(hmd, 0);

	// Adjust camera height to person's height, if available and copy to OVR Vector to calculate projection matrix
	//cameraPosition.y() = ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, cameraPosition.y());
	OVR::Vector3f camPosition(cameraPosition.x(), cameraPosition.y(), cameraPosition.z());

	// Get eye poses, feeding in correct IPD offset
	ovrVector3f viewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset };
	ovrPosef eyeRenderPose[2];
	ovrHmd_GetEyePoses(hmd, 0, viewOffset, eyeRenderPose, NULL);

	// Render for each eye
	ovrGLTexture eyeTexture[2];
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];

		// Bind framebuffer of current eye for off screen rendering
		frameBuffer[eye].bind();

		// Use data from rift sensors
		OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(yaw);
		OVR::Matrix4f finalRollPitchYaw = rollPitchYaw * OVR::Matrix4f(eyeRenderPose[eye].Orientation);
		OVR::Vector3f up = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
		OVR::Vector3f forward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
		OVR::Vector3f shiftedEyePos = camPosition + rollPitchYaw.Transform(eyeRenderPose[eye].Position);

		// Calculate view and projection matrices
		OVR::Matrix4f view = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + forward, up);
		OVR::Matrix4f projection = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], zNear, zFar, ovrProjection_RightHanded);

		// Copy to Eigen matrices (we need column major -> transpose)
		Matrix4f v = Eigen::Map<Matrix4f>((float *)view.Transposed().M);
		Matrix4f p = Eigen::Map<Matrix4f>((float *)projection.Transposed().M);

		// Update matrices
		setProjectionMatrix(p);
		setViewMatrix(v);

		// Draw the mesh for each eye
		PerspectiveRenderer::draw();

		// Do distortion rendering, Present and flush/sync
		OVR::Sizei size(frameBuffer[eye].mSize.x(), frameBuffer[eye].mSize.y());
		eyeTexture[eye].OGL.Header.API = ovrRenderAPI_OpenGL;
		eyeTexture[eye].OGL.Header.TextureSize = size;
		eyeTexture[eye].OGL.Header.RenderViewport.Pos = OVR::Vector2i(0, 0);
		eyeTexture[eye].OGL.Header.RenderViewport.Size = size;
		eyeTexture[eye].OGL.TexId = frameBuffer[eye].getColor();
	}

	// Bind "the" framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// End SDK distortion mode
	ovrHmd_EndFrame(hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
