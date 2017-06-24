#include <imgui.h>
#include "imgui_impl_androidnative_gles3.h"
#include <time.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))




static float        g_ScaleSize = 3.f;
static double       g_Time = 0.0f;
static bool         g_MousePressed[3] = { false, false, false };
static float        g_MouseWheel = 0.0f;
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

void ImGui_ImplAndroidNative_GLES2_RenderDrawLists(ImDrawData* draw_data)
{
	// Backup GL state
	GLint last_program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_array_buffer;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	GLint last_element_array_buffer;
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);

	GLint last_blend_equation_rgb;
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	GLint last_blend_equation_alpha;
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
	GLint last_viewport[4];
	glGetIntegerv(GL_VIEWPORT, last_viewport);

	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	// Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Setup orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	const float ortho_projection[4][4] =
	{
		{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
		{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
		{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
		{ -1.0f,                  1.0f,                   0.0f, 1.0f },
	};

	glUseProgram(g_ShaderHandle);
	glUniform1i(g_AttribLocationTex, 0);
	glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

		for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
		{
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	// Restore modified GL state
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (last_enable_blend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if (last_enable_cull_face)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if (last_enable_depth_test)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (last_enable_scissor_test)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);

	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

static const char* ImGui_ImplAndroidNative_GLES2_GetClipboardText(void* user_data)
{
    //return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplAndroidNative_GLES2_SetClipboardText(void* user_data, const char* text)
{
    //glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void ImGui_ImplAndroidNative_GLES2_MouseButtonCallback(struct AEngine* engine, int button, int action, int /*mods*/)
{
    //if (action == GLFW_PRESS && button >= 0 && button < 3)
    //    g_MousePressed[button] = true;
}

void ImGui_ImplAndroidNative_GLES2_ScrollCallback(struct AEngine* engine, double /*xoffset*/, double yoffset)
{
    //g_MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void ImGui_ImplAndroidNative_GLES2_KeyCallback(struct AEngine* engine, int key, int, int action, int mods)
{
    // ImGuiIO& io = ImGui::GetIO();
    // if (action == GLFW_PRESS)
    //     io.KeysDown[key] = true;
    // if (action == GLFW_RELEASE)
    //     io.KeysDown[key] = false;

    // (void)mods; // Modifiers are not reliable across systems
    // io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    // io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    // io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    // io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void ImGui_ImplAndroidNative_GLES2_CharCallback(struct AEngine* engine, unsigned int c)
{
    // ImGuiIO& io = ImGui::GetIO();
    // if (c > 0 && c < 0x10000)
    //     io.AddInputCharacter((unsigned short)c);
}

bool ImGui_ImplAndroidNative_GLES2_CreateFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	// Build texture atlas
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

															  // Create OpenGL texture
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &g_FontTexture);
	glBindTexture(GL_TEXTURE_2D, g_FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

	glBindTexture(GL_TEXTURE_2D, last_texture);
	return true;
}

bool ImGui_ImplAndroidNative_GLES2_CreateDeviceObjects()
{
	// Backup GL state
	GLint last_texture, last_array_buffer;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

	const GLchar *vertex_shader =
		"precision mediump float;\n"
		"attribute vec2 Position;\n"
		"attribute vec2 UV;\n"
		"attribute vec4 Color;\n"
		"varying vec2 Frag_UV;\n"
		"varying vec4 Frag_Color;\n"
		"uniform mat4 ProjMtx;\n"
		"void main()\n"
		"{\n"
		"	Frag_UV = UV;\n"
		"	Frag_Color = Color;\n"
		"	gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
		"}\n";

	const GLchar* fragment_shader =
		"precision mediump float;\n"
		"varying vec2 Frag_UV;\n"
		"varying vec4 Frag_Color;\n"
		"uniform sampler2D Texture;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = Frag_Color * texture2D( Texture, Frag_UV.st);\n"
		"}\n";

	g_ShaderHandle = glCreateProgram();
	g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
	g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
	glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
	glCompileShader(g_VertHandle);

	GLint logLength;
	glGetShaderiv(g_VertHandle, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = (GLchar *)malloc(logLength);
		glGetShaderInfoLog(g_VertHandle, logLength, &logLength, log);
		
		free(log);
	}

	glCompileShader(g_FragHandle);

	glGetShaderiv(g_FragHandle, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = (GLchar *)malloc(logLength);
		glGetShaderInfoLog(g_FragHandle, logLength, &logLength, log);
		
		free(log);
	}

	glAttachShader(g_ShaderHandle, g_VertHandle);
	glAttachShader(g_ShaderHandle, g_FragHandle);
	glLinkProgram(g_ShaderHandle);

	g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
	g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
	g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
	g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
	g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

	glGenBuffers(1, &g_VboHandle);
	glGenBuffers(1, &g_ElementsHandle);

	glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
	glEnableVertexAttribArray(g_AttribLocationPosition);
	glEnableVertexAttribArray(g_AttribLocationUV);
	glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
	glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

	ImGui_ImplAndroidNative_GLES2_CreateFontsTexture();

	// Restore modified GL state
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);

	return true;
}

void    ImGui_ImplAndroidNative_GLES2_InvalidateDeviceObjects()
{
	if (g_VboHandle)
		glDeleteBuffers(1, &g_VboHandle);
	if (g_ElementsHandle)
		glDeleteBuffers(1, &g_ElementsHandle);
	g_VboHandle = g_ElementsHandle = 0;

	glDetachShader(g_ShaderHandle, g_VertHandle);
	glDeleteShader(g_VertHandle);
	g_VertHandle = 0;

	glDetachShader(g_ShaderHandle, g_FragHandle);
	glDeleteShader(g_FragHandle);
	g_FragHandle = 0;

	glDeleteProgram(g_ShaderHandle);
	g_ShaderHandle = 0;

	if (g_FontTexture)
	{
		glDeleteTextures(1, &g_FontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

bool    ImGui_ImplAndroidNative_GLES2_Init(struct AEngine* engine, bool install_callbacks)
{
    //g_Window = window;

    ImGuiIO& io = ImGui::GetIO();
    // io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    // io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    // io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    // io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    // io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    // io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    // io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    // io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    // io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    // io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    // io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    // io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    // io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    // io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    // io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    // io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    // io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    // io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    // io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn =  ImGui_ImplAndroidNative_GLES2_RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn =  ImGui_ImplAndroidNative_GLES2_SetClipboardText;
    io.GetClipboardTextFn =  ImGui_ImplAndroidNative_GLES2_GetClipboardText;
    //io.ClipboardUserData = g_Window;


    if (install_callbacks)
    {
        // glfwSetMouseButtonCallback(window, ImGui_ImplGlfwGL3_MouseButtonCallback);
        // glfwSetScrollCallback(window, ImGui_ImplGlfwGL3_ScrollCallback);
        // glfwSetKeyCallback(window, ImGui_ImplGlfwGL3_KeyCallback);
        // glfwSetCharCallback(window, ImGui_ImplGlfwGL3_CharCallback);
    }

    return true;
}

void ImGui_ImplAndroidNative_GLES2_Shutdown()
{
    ImGui_ImplAndroidNative_GLES2_InvalidateDeviceObjects();
    ImGui::Shutdown();
}

void ImGui_ImplAndroidNative_GLES2_NewFrame(struct AEngine* engine)
{
    if (!g_FontTexture)
        ImGui_ImplAndroidNative_GLES2_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    float w, h;
    float display_w, display_h;
    //glfwGetWindowSize(g_Window, &w, &h);
   // glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    w=engine->width;
    h=engine->height;
	display_w = w ;
	display_h = h ;
    io.DisplaySize = ImVec2((float)w, (float)h);
    //io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
	io.DisplayFramebufferScale = ImVec2(g_ScaleSize, g_ScaleSize);
    // Setup time step
	double current_time = (double)clock()/100000.;
	io.DeltaTime = current_time-g_Time;//g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    // if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
    // {
    //     double mouse_x, mouse_y;
    //     glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
         io.MousePos = ImVec2((float)engine->state.x, (float)engine->state.y);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    // }
    // else
    // {
    //     io.MousePos = ImVec2(-1,-1);
    // }

    // for (int i = 0; i < 3; i++)
    // {
         io.MouseDown[0] = g_MousePressed[0] ;    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
         //g_MousePressed[0] = false;
    // }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // // Hide OS mouse cursor if ImGui is drawing it
    // glfwSetInputMode(g_Window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}



static int engine_init_display(struct AEngine* engine) {

	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint AttribList[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);


	eglChooseConfig(display, attribs, &config, 1, &numConfigs);


	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, AttribList);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.angle = 0;


	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	//glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);
	//double a = sqrt(4.0);
	return 0;
}


static void engine_draw_frame(struct AEngine* engine) {
	if (engine->display == NULL) {

		return;
	}
	imgui_loop(engine);
	

	

	eglSwapBuffers(engine->display, engine->surface);
}


static void engine_term_display(struct AEngine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}


static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct AEngine* engine = (struct AEngine*)app->userData;

	int32_t eventType = AInputEvent_getType(event);
	switch (eventType) {
	case AINPUT_EVENT_TYPE_MOTION:
		engine->state.x = AMotionEvent_getX(event, 0)/ g_ScaleSize;
		engine->state.y = AMotionEvent_getY(event, 0)/g_ScaleSize+ engine->height/(g_ScaleSize/(g_ScaleSize-1.f));//853.333 1280.1543 1717.f
		switch (AInputEvent_getSource(event)) {
		case AINPUT_SOURCE_TOUCHSCREEN:
			int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
			switch (action) {
			case AMOTION_EVENT_ACTION_DOWN:
				g_MousePressed[0] = true;
				return 1;
				break;
			case AMOTION_EVENT_ACTION_UP:
				g_MousePressed[0] = false;
				return 1;
				break;
			case AMOTION_EVENT_ACTION_MOVE:
				/*engine->state.x = AMotionEvent_getX(event, 0);
				engine->state.y = AMotionEvent_getY(event, 0);*/
				g_MousePressed[0] = g_MousePressed[0];
				return 1;
				break;
			}
			break;
		} // end switch
		break;
	case AINPUT_EVENT_TYPE_KEY:
		// handle key input...
		break;
	} // end switch
	g_MousePressed[0] = false;
	return 0;
}


static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct AEngine* engine = (struct AEngine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		//系统已经要求我们保存当前状态。就这样做。
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		//正在显示窗口，让其准备就绪。
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			ImGui_ImplAndroidNative_GLES2_Init(engine, true);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		//正在隐藏或关闭窗口，请其进行清理。
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		
		break;
	case APP_CMD_LOST_FOCUS:
		//当我们的应用程序失去焦点时，我们会停止监控加速计。
		//这可在不使用时避免使用电池。
		
		//另外，停止动画。
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}


void android_main(struct android_app* state) {
	struct AEngine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

//	imgui_init(&engine);

	if (state->savedState != NULL) {
		//我们从之前保存的状态开始；从它还原。
		engine.state = *(struct saved_state*)state->savedState;
	}

	engine.animating = 1;
	


	while (1) {

		int ident;
		int events;
		struct android_poll_source* source;

		
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {
			if (source != NULL) {
				source->process(state, source);
			}
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}
			engine_draw_frame(&engine);
	}
}
