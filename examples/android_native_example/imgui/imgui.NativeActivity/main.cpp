#include "imgui.h"
#include "imgui_impl_androidnative_gles3.h"
#include "native_keyboard.h"
//void imgui_init(struct AEngine* engine) {
//
//}

void imgui_loop(struct AEngine* engine) {
	static bool show_test_window = true;
	static bool show_another_window = false;
	static bool show_keyboard = false;
	static ImVec4 clear_color = ImColor(114, 144, 154);
	ImGui_ImplAndroidNative_GLES2_NewFrame(engine);
	{
		ImGui::SetNextWindowPos(ImVec2(0, 1500), ImGuiSetCond_FirstUseEver);
		static float f = 0.0f;
		ImGui::Text("%lf", (double)clock() / 100000.);
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		if (ImGui::Button("Keyboard")) show_keyboard ^=1;
		
		showKeyboard(engine->app->activity,show_keyboard);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
	//if (show_another_window)
	//{
	//	ImGui::SetNextWindowPos(ImVec2(0, 1500), ImGuiSetCond_FirstUseEver);
	//	ImGui::Begin("Another Window", &show_another_window);
	//	ImGui::Text("Hello");
	//	ImGui::End();
	//}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	//if (show_test_window)
	//{
	//	ImGui::SetNextWindowPos(ImVec2(0, 1500), ImGuiSetCond_FirstUseEver);
	//	ImGui::ShowTestWindow(&show_test_window);
	//}

	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
}