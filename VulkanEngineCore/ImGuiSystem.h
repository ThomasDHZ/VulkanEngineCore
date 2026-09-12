#pragma once
#include "DLL.h"
#include "Platform.h"
#include "VulkanWindow.h"
#include <imgui_impl_glfw.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

class ImGuiSystem
{
public:
	static ImGuiSystem& Get();

private:
	ImGuiSystem() = default;
	~ImGuiSystem() = default;
	ImGuiSystem(const ImGuiSystem&) = delete;
	ImGuiSystem& operator=(const ImGuiSystem&) = delete;
	ImGuiSystem(ImGuiSystem&&) = delete;
	ImGuiSystem& operator=(ImGuiSystem&&) = delete;

	VkRenderPass		  m_renderPass = VK_NULL_HANDLE;
	VkDescriptorPool	  m_imGuiDescriptorPool = VK_NULL_HANDLE;
	Vector<VkFramebuffer> m_swapChainFramebuffers;

	void				  CreateRenderPass();
	void				  CreateDescriptorPool();
	void				  CreateRendererFramebuffers();
	static void			  ImGuiResult(VkResult err);

public:
	CORE_DLL_EXPORT void				  StartUp();
	CORE_DLL_EXPORT void				  StartFrame();
	CORE_DLL_EXPORT void				  EndFrame();
	CORE_DLL_EXPORT void				  Draw(VkCommandBuffer& commandBuffer);
	CORE_DLL_EXPORT void				  RebuildSwapChain();
	CORE_DLL_EXPORT void				  Destroy();

	CORE_DLL_EXPORT void				  FpsDisplay();
	CORE_DLL_EXPORT void				  VulkanDebugger();

	CORE_DLL_EXPORT bool				  SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT bool				  SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT bool				  SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT bool				  SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	CORE_DLL_EXPORT 	void				  Text(const char* fmt, ...) IM_FMTARGS(2);
	CORE_DLL_EXPORT 	void				  Separator();
	CORE_DLL_EXPORT 	void				  SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);
	CORE_DLL_EXPORT 	void				  NewLine();
	CORE_DLL_EXPORT 	bool				  Button(const char* label, const ImVec2& size = ImVec2(0, 0));
	CORE_DLL_EXPORT 	bool				  Checkbox(const char* label, bool* v);
	CORE_DLL_EXPORT 	bool				  InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	CORE_DLL_EXPORT 	bool				  InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputFloat2(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputFloat3(const char* label, float v[3], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputFloat4(const char* label, float v[4], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  IsKeyDown(ImGuiKey key);
	CORE_DLL_EXPORT 	bool				  IsKeyPressed(ImGuiKey key, bool repeat = true);
	CORE_DLL_EXPORT 	bool				  IsKeyReleased(ImGuiKey key);
	CORE_DLL_EXPORT bool				  IsItemActive();
	CORE_DLL_EXPORT bool				  IsItemFocused();
	CORE_DLL_EXPORT bool				  Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
	CORE_DLL_EXPORT 	bool				  BeginChild(const char* str_id, const ImVec2& size, bool border, ImGuiWindowFlags flags);
	CORE_DLL_EXPORT void				  TextColored(const ImVec4& col, const char* fmt, ...);
	CORE_DLL_EXPORT void				  TextWrapped(const char* fmt, ...);
	CORE_DLL_EXPORT float				  GetScrollY();
	CORE_DLL_EXPORT float				  GetScrollMaxY();
	CORE_DLL_EXPORT void				  SetScrollHereY(float center_y_ratio);
	CORE_DLL_EXPORT void				  EndChild();
	CORE_DLL_EXPORT void				  PushItemWidth(float item_width);
	CORE_DLL_EXPORT void				  PopItemWidth();
	CORE_DLL_EXPORT bool				  IsWindowAppearing();
	CORE_DLL_EXPORT void				  SetKeyboardFocusHere(int offset);
	CORE_DLL_EXPORT void				  End();
	CORE_DLL_EXPORT float				  GetFrameHeightWithSpacing();
};
CORE_DLL_EXPORT extern ImGuiSystem& imGuiSystem;
inline ImGuiSystem& ImGuiSystem::Get()
{
	static ImGuiSystem instance;
	return instance;
}