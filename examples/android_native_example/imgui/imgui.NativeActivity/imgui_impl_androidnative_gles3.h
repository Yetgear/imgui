
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};


struct AEngine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};
extern void imgui_init(struct AEngine* engine);
extern void imgui_loop(struct AEngine* engine);
IMGUI_API bool         ImGui_ImplAndroidNative_GLES2_Init(struct AEngine* engine, bool install_callbacks);
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_Shutdown();
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_NewFrame(struct AEngine* engine);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_InvalidateDeviceObjects();
IMGUI_API bool         ImGui_ImplAndroidNative_GLES2_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks.
// You can also handle inputs yourself and use those as a reference.
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_MouseButtonCallback(struct AEngine* engine, int button, int action, int mods);
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_ScrollCallback(struct AEngine* engine, double xoffset, double yoffset);
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_KeyCallback(struct AEngine* engine, int key, int scancode, int action, int mods);
IMGUI_API void         ImGui_ImplAndroidNative_GLES2_CharCallback(struct AEngine* engine, unsigned int c);
