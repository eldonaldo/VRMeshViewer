#include "renderer/RiftRenderer.hpp"

VR_NAMESPACE_BEGIN

RiftRenderer::RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: PerspectiveRenderer(shader, fov, width, height, zNear, zFar) {

	// Reset all
	setViewMatrix(Matrix4f::Identity());
	setProjectionMatrix(Matrix4f::Identity());

	// Leap passthrough shader
	leapShader = std::make_shared<GLShader>();
	leapShader->initFromFiles("leap-shader", "resources/shader/passthrough-vertex-shader.glsl", "resources/shader/passthrough-fragment-shader.glsl");
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

	// Do distortion rendering, Present and flush/sync
	for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
		ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];
		OVR::Sizei size(frameBuffer[eye].mSize.x(), frameBuffer[eye].mSize.y());
		eyeTexture[eye].OGL.Header.API = ovrRenderAPI_OpenGL;
		eyeTexture[eye].OGL.Header.TextureSize = size;
		eyeTexture[eye].OGL.Header.RenderViewport.Pos = OVR::Vector2i(0, 0);
		eyeTexture[eye].OGL.Header.RenderViewport.Size = size;
		eyeTexture[eye].OGL.TexId = frameBuffer[eye].getColor();
	}

	// Dismiss warning
	ovrHmd_DismissHSWDisplay(hmd);

	// Leap image textures
	leapRawLeftTexture = GLFramebuffer::createTexture();
	leapRawRightTexture = GLFramebuffer::createTexture();
	leapDistortionLeftTexture = GLFramebuffer::createTexture();
	leapDistortionRightTexture = GLFramebuffer::createTexture();

	// Upload leap shader data
	leapShader->bind();
	uploadBackgroundCube();
}

void RiftRenderer::update (Matrix4f &s, Matrix4f &r, Matrix4f &t) {
	// Update global state
	PerspectiveRenderer::update(s, r, t);

	// Leap passthrough
	Leap::Frame frame = leapController.frame();
	if (frame.isValid()) {
		Leap::Image left = frame.images()[0];
		Leap::Image right = frame.images()[1];

		// Check images sizes
		if (left.width() != leapImageWidth || left.height() != leapImageHeight || left.distortionWidth() != leapDistortionWidth || left.distortionHeight() != leapDistortionHeight) {
			// Left
			glBindTexture(GL_TEXTURE_2D, leapRawLeftTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, left.width(), left.height(), 0, GL_RED, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D, leapDistortionLeftTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, left.distortionWidth() / 2, left.distortionHeight(), 0, GL_RG, GL_FLOAT, 0);

			// Right
			glBindTexture(GL_TEXTURE_2D, leapRawRightTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, right.width(), right.height(), 0, GL_RED, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D, leapDistortionRightTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, right.distortionWidth() / 2, right.distortionHeight(), 0, GL_RG, GL_FLOAT, 0);

			leapImageWidth = left.width(); leapImageHeight = left.height();
			leapDistortionWidth = left.distortionWidth(); leapDistortionHeight = left.distortionHeight();
		}

		// Update image and distortion textures
		if (left.width() > 0) {
			glBindTexture(GL_TEXTURE_2D, leapRawLeftTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, left.width(), left.height(), GL_RED, GL_UNSIGNED_BYTE, left.data());
			glBindTexture(GL_TEXTURE_2D, leapDistortionLeftTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, left.distortionWidth() / 2, left.distortionHeight(), GL_RG, GL_FLOAT, left.distortion());
		}

		if (right.width() > 0) {
			glBindTexture(GL_TEXTURE_2D, leapRawRightTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, right.width(), right.height(), GL_RED, GL_UNSIGNED_BYTE, right.data());
			glBindTexture(GL_TEXTURE_2D, leapDistortionRightTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, right.distortionWidth() / 2, right.distortionHeight(), GL_RG, GL_FLOAT, right.distortion());
		}
	}
}

void RiftRenderer::clear (Vector3f background) {
	frameBuffer[0].clear();
	frameBuffer[1].clear();
}

void RiftRenderer::draw () {
	// Heads yaw angle
	static float yaw = 0.f;

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

		// Leap passthrough
		leapShader->bind();
		drawOnCube(eye);

		// Draw the mesh for each eye
		shader->bind();
		PerspectiveRenderer::draw();
	}

	// Bind "the" framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// End SDK distortion mode
	ovrHmd_EndFrame(hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void RiftRenderer::uploadBackgroundCube() {
	GLfloat maxZ = 1.f - std::numeric_limits<GLfloat>::epsilon();

	// Interleaved positions an tex coords
	GLfloat quad[] = {
		-1.f, -1.f, maxZ, 1.f, // v0
		 0.f,  0.f, 0.f,  0.f, // t0
		 1.f, -1.f, maxZ, 1.f, // ...
		 1.f,  0.f, 0.f,  0.f,
		 1.f,  1.f, maxZ, 1.f,
		 1.f,  1.f, 0.f,  0.f,
		-1.f,  1.f, maxZ, 1.f,
		 0.f,  1.f, 0.f,  0.f
	};

	GLubyte indices[] = { 0, 1, 2, 0, 2, 3 };

	// Generate and bind a VAO
	glGenVertexArrays(1, &leapVAO);
	glBindVertexArray(leapVAO);

	// Setup VBO
	glGenBuffers(1, &leapVBO);
	glBindBuffer(GL_ARRAY_BUFFER, leapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	// Setup index buffer
	glGenBuffers(1, &leapIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leapIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Ins
	GLuint pos = glGetAttribLocation(leapShader->getId(), "position");
	glVertexAttribPointer(pos, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(pos);

	GLuint tex = glGetAttribLocation(leapShader->getId(), "texCoord");
	glVertexAttribPointer(tex, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	glEnableVertexAttribArray(tex);

	// Texture uniforms
	GLuint rawSampler = glGetUniformLocation(leapShader->getId(), "rawTexture");
	GLuint distortionSampler = glGetUniformLocation(leapShader->getId(), "distortionTexture");
	glUniform1i(rawSampler, /*GL_TEXTURE*/0);
	glUniform1i(distortionSampler,/*GL_TEXTURE*/1);

	// Reset state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void RiftRenderer::drawOnCube(ovrEyeType eye) {
	if (eye != 0) {
		// Left
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, leapRawLeftTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, leapDistortionLeftTexture);
	} else {
		// Right
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, leapRawRightTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, leapDistortionRightTexture);
	}
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();
}

VR_NAMESPACE_END
