#include "renderer/RiftRenderer.hpp"

VR_NAMESPACE_BEGIN

RiftRenderer::RiftRenderer (std::shared_ptr<GLShader> &shader, float fov, float width, float height, float zNear, float zFar)
	: leapShader(nullptr), leapVAO(0), leapV_VBO(0), leapUV_VBO(0), leapF_VBO(0)
	, PerspectiveRenderer(shader, fov, width, height, zNear, zFar) {

	// Leap passthrough shader
	if (Settings::getInstance().LEAP_USE_PASSTHROUGH) {
		leapShader = std::make_shared<GLShader>();
		leapShader->init(
			"leap-shader",

			// Vertex shader
			std::string("#version 330") + "\n" +

			"uniform mat4 mvp;" + "\n" +

			"in vec3 position;" + "\n" +
			"in vec2 texCoord;" + "\n" +

			"out vec2 uv;" + "\n" +

			"void main () {" + "\n" +
			"	uv = texCoord;" + "\n" +
			"	gl_Position = mvp * vec4(position, 1.0);" + "\n" +
			"}" + "\n",

			// Fragment shader
			std::string("#version 330") + "\n" +

			"uniform sampler2D rawTexture;" + "\n" +
			"uniform sampler2D distortionTexture;" + "\n" +

			"in vec2 uv;" + "\n" +

			"out vec4 color;" + "\n" +

			"void main () {" + "\n" +
			"	vec4 index = texture(distortionTexture, uv);" + "\n" +

			"	// Only use xy within [0, 1]" + "\n" +
			"	if (index.r > 0.0 && index.r < 1.0 && index.g > 0.0 && index.g < 1.0)" + "\n" +
			"    	color = vec4(texture(rawTexture, index.rg).rrr, 1);" + "\n" +
			"	else" + "\n" +
			"    	color = vec4(0, 0, 0, 1);" + "\n" +
			"}" + "\n"
		);
	}
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
	// Need to attach viewerGLFWwindow for direct rendering (only supported on windows)
	cfg.OGL.Window = glfwGetWin32Window(viewerGLFWwindow);
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
	
	// Upload leap shader data
	if (Settings::getInstance().LEAP_USE_PASSTHROUGH) {
		if (!leapController.isConnected())
			throw VRException("Passthrough enabled but Leap not connected");

		leapShader->bind();

		// Leap image textures
		// Left
		std::tuple<GLuint, GLuint, GLuint> t0 = GLFramebuffer::createPBOTexture(rawWidth, rawHeight, 1, 1);
		leapRawTexture[0] = std::get<0>(t0);

		std::tuple<GLuint, GLuint, GLuint> t1 = GLFramebuffer::createPBOTexture(distWidth, distHeight, 1, 8, false);
		leapDistortionTexture[0] = std::get<0>(t1);

		// Right
		std::tuple<GLuint, GLuint, GLuint> t2 = GLFramebuffer::createPBOTexture(rawWidth, rawHeight, 1, 1);
		leapRawTexture[1] = std::get<0>(t2);
		
		std::tuple<GLuint, GLuint, GLuint> t3 = GLFramebuffer::createPBOTexture(distWidth, distHeight, 1, 8, false);
		leapDistortionTexture[1] = std::get<0>(t3);

		// Left PBOs
		leap_PBO[0][0][0] = std::get<1>(t0);
		leap_PBO[0][0][1] = std::get<1>(t1);
		leap_PBO[0][1][0] = std::get<2>(t0);
		leap_PBO[0][1][1] = std::get<2>(t1);

		// Right PBOs
		leap_PBO[1][0][0] = std::get<1>(t2);
		leap_PBO[1][0][1] = std::get<1>(t3);
		leap_PBO[1][1][0] = std::get<2>(t2);
		leap_PBO[1][1][1] = std::get<2>(t3);

		// Upload geometry
		uploadBackgroundCube();
	}
}

void RiftRenderer::update (Matrix4f &s, Matrix4f &r, Matrix4f &t) {
	// Update global state
	PerspectiveRenderer::update(s, r, t);
		
	if (Settings::getInstance().LEAP_USE_PASSTHROUGH) {
		if (Settings::getInstance().LEAP_USE_LISTENER && leapController.hasFocus())
			frame = leapController.frame();

		Leap::Image left = frame.images()[0], right = frame.images()[1];
			
		// Indices for PBOs
		static int index = 1;
		static int nextIndex = 0;
		index = (index + 1) % 2;
		nextIndex = (index + 1) % 2;

		// Buffer sizes
		int bufferSizeRaw = 153600;
		int bufferSizeDistortion = 32768;

		/**
		* For left and right camera images
		*
		* We're using pixel buffer objects to increase performance.
		* Per texture two PBO are used. On frame n we draw on the n % 2 PBO
		* and displaz the content of the (n - 1) % 2 PBO. That wat we create from
		* the synchronous glTexImage/glTexSubImage a pipelined ansync mode.
		*/
		for (int i = 0; i < 2; i++) {
			Leap::Image &currentImage = left;
			if (i == 1)
				currentImage = right;

			// =======================
			// DRAW RAW TEXTURE
			// =======================

			// Bind the texture and PBO
			glBindTexture(GL_TEXTURE_2D, leapRawTexture[i]);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, leap_PBO[i][index][0]);

			// Copy pixels from PBO to texture object. Use offset instead of ponter.
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rawWidth, rawHeight, GL_RED, GL_UNSIGNED_BYTE, 0);

			// =======================
			// UPDATE RAW TEXTURE
			// =======================

			// Bind PBO to update texture source
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, leap_PBO[i][nextIndex][0]);

			// Note that glMapBufferARB() causes sync issue.
			// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
			// until GPU to finish its job. To avoid waiting (idle), you can call
			// first glBufferDataARB() with NULL pointer before glMapBufferARB().
			// If you do that, the previous data in PBO will be discarded and
			// glMapBufferARB() returns a new allocated pointer immediately
			// even if GPU is still working with the previous data.
			glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferSizeRaw, 0, GL_STREAM_DRAW);

			// Map the buffer object into client's memory
			GLubyte* ptr1 = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
			if (ptr1) {
				memcpy(ptr1, currentImage.data(), bufferSizeRaw); // Update data directly on the mapped buffer
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // Release the mapped buffer
			}
			// =======================
			// DRAW DISTORTION TEXTURE
			// =======================

			// Bbind the texture and PBO
			glBindTexture(GL_TEXTURE_2D, leapDistortionTexture[i]);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, leap_PBO[i][index][1]);

			// Copy pixels from PBO to texture object. Use offset instead of ponter.
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, distWidth, distHeight, GL_RG, GL_FLOAT, 0);

			// =======================
			// UPDATE DISTORTION TEXTURE
			// =======================

			// Bind PBO to update texture source
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, leap_PBO[i][nextIndex][1]);

			// Note that glMapBufferARB() causes sync issue.
			// If GPU is working with this buffer, glMapBufferARB() will wait(stall)
			// until GPU to finish its job. To avoid waiting (idle), you can call
			// first glBufferDataARB() with NULL pointer before glMapBufferARB().
			// If you do that, the previous data in PBO will be discarded and
			// glMapBufferARB() returns a new allocated pointer immediately
			// even if GPU is still working with the previous data.
			glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferSizeDistortion, 0, GL_STREAM_DRAW);

			// Map the buffer object into client's memory
			GLfloat* ptr2 = (GLfloat*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
			if (ptr2) {
				memcpy(ptr2, currentImage.distortion(), bufferSizeDistortion); // Ipdate data directly on the mapped buffer
				glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // Release the mapped buffer
			}
		}

		// Release PBO buffer
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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
			
			// Offset for the corresponding leap cameras
			float mid = 0.5f;
			float x = (eye == ovrEyeType::ovrEye_Right ? -1.f : +1.f) * Settings::getInstance().LEAP_CAMERA_SHIFT_X;
			OVR::Vector3f eyeOffset(x, 0.f, 0.5f * Settings::getInstance().LEAP_CAMERA_SHIFT_Z);
			OVR::Vector3f leapCam = shiftedEyePos + eyeOffset;

			// Each has its own view matrix
			OVR::Matrix4f viewLeapCam = OVR::Matrix4f::LookAtRH(leapCam, leapCam + forward, up);
			Matrix4f vl = Eigen::Map<Matrix4f>((float *)viewLeapCam.Transposed().M);
			setViewMatrixLeap(vl);

		if (Settings::getInstance().LEAP_USE_PASSTHROUGH) {
			// Draw Leap distorted image
			leapShader->bind();
			leapShader->setUniform("mvp", getProjectionMatrix());
			drawOnCube(eye);
		}

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
	// Vertices. The corners must be at position 4 for the distortion correction to work!
	GLfloat maxZ = -1.f;

	MatrixXf V(3, 4);
	V.col(0) = Vector3f(-4.f, -4.f, maxZ);
	V.col(1) = Vector3f(4.f, -4.f, maxZ);
	V.col(2) = Vector3f(4.f, 4.f, maxZ);
	V.col(3) = Vector3f(-4.f, 4.f, maxZ);

	// UV coords
	MatrixXf UV(2, 4);
	UV.col(0) = Vector2f(0.f, 0.f);
	UV.col(1) = Vector2f(1.f, 0.f);
	UV.col(2) = Vector2f(1.f, 1.f);
	UV.col(3) = Vector2f(0.f, 1.f);
	
	// Indices
	MatrixXu F(3, 2);
	F.col(0) = Vector3ui(0, 1, 2);
	F.col(1) = Vector3ui(0, 2, 3);

	// VAO
	glGenVertexArrays(1, &leapVAO);
	glBindVertexArray(leapVAO);

	// Positions
	glGenBuffers(1, &leapV_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, leapV_VBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * V.cols() * sizeof(GLfloat), (const uint8_t *)V.data(), GL_DYNAMIC_DRAW);
	GLuint pp = glGetAttribLocation(leapShader->getId(), "position");
	glVertexAttribPointer(pp, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pp);

	// UV
	glGenBuffers(1, &leapUV_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, leapUV_VBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * V.cols() * sizeof(GLfloat), (const uint8_t *)UV.data(), GL_DYNAMIC_DRAW);
	GLuint uvp = glGetAttribLocation(leapShader->getId(), "texCoord");
	glVertexAttribPointer(uvp, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(uvp);

	// Indices
	glGenBuffers(1, &leapF_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leapF_VBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * F.cols() * sizeof(GLuint), (const uint8_t *)F.data(), GL_DYNAMIC_DRAW);

	// Texture uniforms
	GLuint rawSampler = glGetUniformLocation(leapShader->getId(), "rawTexture");
	GLuint distortionSampler = glGetUniformLocation(leapShader->getId(), "distortionTexture");
	glUniform1i(rawSampler, /*GL_TEXTURE*/0);
	glUniform1i(distortionSampler,/*GL_TEXTURE*/1);

	// Reset state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RiftRenderer::drawOnCube(ovrEyeType eye) {
	glBindVertexArray(leapVAO);

	// Left or right
	int i = eye == ovrEyeType::ovrEye_Left ? 0 : 1;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, leapRawTexture[i]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, leapDistortionTexture[i]);
	
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

void RiftRenderer::cleanUp () {
	PerspectiveRenderer::cleanUp();

	if (Settings::getInstance().LEAP_USE_PASSTHROUGH) {
		glDeleteBuffers(1, &leapV_VBO);
		glDeleteBuffers(1, &leapUV_VBO);
		glDeleteBuffers(1, &leapF_VBO);
		glDeleteVertexArrays(1, &leapVAO);
	}
}

VR_NAMESPACE_END
