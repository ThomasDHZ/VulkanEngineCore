#pragma once
#include "Platform.h"
#include "VulkanWindow.h"
#include <imgui_impl_glfw.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

class DLL_EXPORT ImGuiSystem
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
	void				  StartUp();
	void				  StartFrame();
	void				  EndFrame();
	void				  Draw(VkCommandBuffer& commandBuffer);
	void				  RebuildSwapChain();
	void				  Destroy();

	void				  FpsDisplay();
	void				  VulkanDebugger();

	bool				  SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	bool				  SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	bool				  SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	bool				  SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	bool				  SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	bool				  SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	bool				  SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	bool				  SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	void				  Text(const char* fmt, ...) IM_FMTARGS(2);
	void				  Separator();
	void				  SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);
	void				  NewLine();
	bool				  Button(const char* label, const ImVec2& size = ImVec2(0, 0));
	bool				  InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	bool				  InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	bool				  InputFloat2(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	bool				  InputFloat3(const char* label, float v[3], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	bool				  InputFloat4(const char* label, float v[4], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	bool				  InputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
	bool				  InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags = 0);
	bool				  InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags = 0);
	bool				  InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags = 0);
	bool				  InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
	bool				  IsKeyDown(ImGuiKey key);
	bool				  IsKeyPressed(ImGuiKey key, bool repeat = true);                  
	bool				  IsKeyReleased(ImGuiKey key);
	bool				  IsItemActive();
	bool				  IsItemFocused();
	bool				  Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
	bool				  BeginChild(const char* str_id, const ImVec2& size, bool border, ImGuiWindowFlags flags);
	void				  TextColored(const ImVec4& col, const char* fmt, ...);
	void				  TextWrapped(const char* fmt, ...);
	float				  GetScrollY();
	float				  GetScrollMaxY();
	void				  SetScrollHereY(float center_y_ratio);
	void				  EndChild();
	void				  PushItemWidth(float item_width);
	void				  PopItemWidth();
	bool				  IsWindowAppearing();
	void				  SetKeyboardFocusHere(int offset);
	void				  End();
	float				  GetFrameHeightWithSpacing();
};
extern DLL_EXPORT ImGuiSystem& imGuiSystem;
inline ImGuiSystem& ImGuiSystem::Get()
{
	static ImGuiSystem instance;
	return instance;
}