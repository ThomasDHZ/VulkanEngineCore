#pragma once
#include "Platform.h"
#include "VulkanWindow.h"
#include <imgui_impl_glfw.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

class ImGuiRenderer
{
public:
	static ImGuiRenderer& Get();

private:
	ImGuiRenderer() = default;
	~ImGuiRenderer() = default;
	ImGuiRenderer(const ImGuiRenderer&) = delete;
	ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;
	ImGuiRenderer(ImGuiRenderer&&) = delete;
	ImGuiRenderer& operator=(ImGuiRenderer&&) = delete;

	VkRenderPass		  m_renderPass = VK_NULL_HANDLE;
	VkDescriptorPool	  m_imGuiDescriptorPool = VK_NULL_HANDLE;
	Vector<VkFramebuffer> m_swapChainFramebuffers;

	VkRenderPass		  CreateRenderPass();
	Vector<VkFramebuffer> CreateRendererFramebuffers(const VkRenderPass& renderPass);
	static void			  ImGuiResult(VkResult err);
public:
	DLL_EXPORT void				  StartUp();
	DLL_EXPORT void				  StartFrame();
	DLL_EXPORT void				  EndFrame();
	DLL_EXPORT void				  Draw(VkCommandBuffer& commandBuffer);
	DLL_EXPORT void				  RebuildSwapChain();
	DLL_EXPORT void				  Destroy();

	DLL_EXPORT void			 FpsDisplay();
	DLL_EXPORT bool          SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	DLL_EXPORT bool          SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	DLL_EXPORT bool          SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	DLL_EXPORT bool          SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	DLL_EXPORT bool          SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
	DLL_EXPORT bool          SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	DLL_EXPORT bool          SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	DLL_EXPORT bool          SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

};
extern DLL_EXPORT ImGuiRenderer& imGuiSystem;
inline ImGuiRenderer& ImGuiRenderer::Get()
{
	static ImGuiRenderer instance;
	return instance;
}