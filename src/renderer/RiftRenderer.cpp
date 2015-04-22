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
	cfg.OGL.Header.Multisample = 0;
#if defined(PLATFORM_WINDOWS)
	// Need to attach window for direct rendering (only supported on windows)
	cfg.OGL.Window = glfwGetWin32Window(window);
	if (!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
		ovrHmd_AttachToWindow(hmd, glfwGetWin32Window(window), NULL, NULL);
#endif

	// We use distortion rendering
	ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive, eyeFov, eyeRenderDesc);
	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
}

void RiftRenderer::update () {
	PerspectiveRenderer::update();
}

void RiftRenderer::clear (Vector3f background) {
	PerspectiveRenderer::clear(background);
}

void RiftRenderer::draw () {

	// Begin Rift SDK distortion mode
	ovrFrameTiming frameTiming = ovrHmd_BeginFrame(hmd, 0);

	// Get eye poses, feeding in correct IPD offset
	ovrVector3f viewOffset[2] = {eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset};
	ovrPosef eyeRenderPose[2];
	ovrHmd_GetEyePoses(hmd, 0, viewOffset, eyeRenderPose, NULL);

	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];

		// Bind framebuffer of current eye for off screen rendering
		frameBuffer[eyeIndex].bind();
		frameBuffer[eyeIndex].clear();

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

		// Update shader state
		shader->bind();
		shader->setUniform("light.position", Vector3f(0, 0, 5));
		shader->setUniform("modelMatrix", getModelMatrix());
		shader->setUniform("normalMatrix", getNormalMatrix());
		shader->setUniform("mvp", getMvp());

		// Draw the mesh for each eye
		shader->drawIndexed(GL_TRIANGLES, 0, mesh->getNumFaces());

		// Do distortion rendering, Present and flush/sync
		OVR::Sizei size(frameBuffer[eyeIndex].mSize.x(), frameBuffer[eyeIndex].mSize.y());
		eyeTexture[eyeIndex].OGL.Header.API = ovrRenderAPI_OpenGL;
		eyeTexture[eyeIndex].OGL.Header.TextureSize = size;
		eyeTexture[eyeIndex].OGL.Header.RenderViewport.Pos = OVR::Vector2i(0, 0);
		eyeTexture[eyeIndex].OGL.Header.RenderViewport.Size = size;
		eyeTexture[eyeIndex].OGL.TexId = frameBuffer[eyeIndex].getColor();
	}

	// End SDK distortion mode
	ovrHmd_EndFrame(hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
