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
		throw new VRException("HMD not set! Can't do pre necessary processing for the Rift");

	// Configure Stereo settings.
	ovrFovPort eyeFov[2] = { hmd->DefaultEyeFov[0], hmd->DefaultEyeFov[1] };

	// Generate framebuffers for left and right eye
	OVR::Sizei texLeft = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, eyeFov[0], 1.0f);
	frameBuffer[0].init(Vector2i(texLeft.w, texLeft.h), 0, true);
	frameBuffer[0].release();

	OVR::Sizei texRight = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, eyeFov[1], 1.0f);
	frameBuffer[1].init(Vector2i(texRight.w, texRight.h), 0, true);
	frameBuffer[1].release();

	// Configure the Rift to use OpenGL
	cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	cfg.OGL.Header.BackBufferSize = hmd->Resolution;
	cfg.OGL.Header.Multisample = 1;
#if defined(PLATFORM_WINDOWS)
	// Need to attach window for direct rendering (only supported on windows)
	cfg.OGL.Window = glfwGetWin32Window(window);
	cfg.OGL.DC = GetDC(cfg.OGL.Window);
#endif

	// We use distortion rendering
	ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive, eyeFov, eyeRenderDesc);
	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);

	// Dismiss warning
	ovrHmd_DismissHSWDisplay(hmd);
}

void RiftRenderer::update () {
	//PerspectiveRenderer::update();
}

void RiftRenderer::clear (Vector3f background) {
	frameBuffer[0].clear();
	frameBuffer[1].clear();
}

void RiftRenderer::draw () {
	// Camera position
	Vector3f camPosition(0, 0, 5);
	
	// Begin distortion rendering
	ovrFrameTiming frameTiming = ovrHmd_BeginFrame(hmd, 0);

	// Adjust camera height to person's height, if available
	camPosition.y() = ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, camPosition.y());

	// Copy to OVR Vector to calculate projection matrix
	OVector3f cameraPosition(camPosition.x(), camPosition.y(), camPosition.z());

	// We start out looking in the positive Z (180 degree rotation).
	float yaw = 3.141592f;

	// Get eye poses, feeding in correct IPD offset
	ovrVector3f viewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset };
	ovrPosef eyeRenderPose[2];
	ovrHmd_GetEyePoses(hmd, 0, viewOffset, eyeRenderPose, NULL);

	// Render for each eye
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];

		// Bind framebuffer of current eye for off screen rendering
		frameBuffer[eyeIndex].bind();

		// Use data from rift sensors
		//OMatrix4f rollPitchYaw       = OMatrix4f::RotationY(yaw);
		//OMatrix4f finalRollPitchYaw  = rollPitchYaw * OMatrix4f(eyeRenderPose[eye].Orientation);
		//OVector3f finalUp            = finalRollPitchYaw.Transform(OVector3f(0, 1, 0));
		//OVector3f finalForward       = finalRollPitchYaw.Transform(OVector3f(0, 0, -1));
		//OVector3f shiftedEyePos      = cameraPosition + rollPitchYaw.Transform(eyeRenderPose[eye].Position);

		//OMatrix4f view = OMatrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
		//OMatrix4f projection = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], zNear, zFar, ovrProjection_RightHanded);

		//// Copy to Eigen matrices
		//Matrix4f v = Eigen::Map<Matrix4f>((float *) view.M);
		//Matrix4f p = Eigen::Map<Matrix4f>((float *) projection.M);

		//// Update matrices
		//setViewMatrix(v);
		//setProjectionMatrix(p);

		setProjectionMatrix(frustum(-fW, fW, -fH, fH, zNear, zFar));
		setViewMatrix(lookAt(
			camPosition, // Camera is at (0, 0, 5), in world space
			Vector3f(0, 0, 0), // And looks at the origin
			Vector3f(0, 1, 0) // Head is up
		));

		// Update shader state
		shader->bind();
		shader->setUniform("light.position", camPosition);
		shader->setUniform("modelMatrix", getModelMatrix());
		shader->setUniform("normalMatrix", getNormalMatrix());
		shader->setUniform("mvp", getMvp());

		// Draw the mesh for each eye
		shader->drawIndexed(GL_TRIANGLES, 0, mesh->getNumFaces());
	}

	// Do distortion rendering, Present and flush/sync
	ovrGLTexture eyeTexture[2];
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		OVR::Sizei size(frameBuffer[eyeIndex].mSize.x(), frameBuffer[eyeIndex].mSize.y());
		eyeTexture[eyeIndex].OGL.Header.API = ovrRenderAPI_OpenGL;
		eyeTexture[eyeIndex].OGL.Header.TextureSize = size;
		eyeTexture[eyeIndex].OGL.Header.RenderViewport.Pos = OVR::Vector2i(0, 0);
		eyeTexture[eyeIndex].OGL.Header.RenderViewport.Size = size;
		eyeTexture[eyeIndex].OGL.TexId = frameBuffer[eyeIndex].getColor();
	}

	// Back to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// End SDK distortion mode
	ovrHmd_EndFrame(hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
