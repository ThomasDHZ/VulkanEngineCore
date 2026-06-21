#pragma once
#include "Platform.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanDebugger.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

class VulkanSystem
{
	public:
		static VulkanSystem& Get();

	private:
		VulkanSystem() = default;
		~VulkanSystem() = default;
		VulkanSystem(const VulkanSystem&) = delete;
		VulkanSystem& operator=(const VulkanSystem&) = delete;
		VulkanSystem(VulkanSystem&&) = delete;
		VulkanSystem& operator=(VulkanSystem&&) = delete;

		void*										m_windowHandle = nullptr;
		VulkanInstance								m_instance;
		VulkanDebugger								m_debug;
		VulkanDevice								m_device;
		VulkanSwapchain								m_swapChain;
		VulkanCommandBuffer							m_commandBuffer;

	public:
		DLL_EXPORT void								RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize);
		DLL_EXPORT uint32							GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
		DLL_EXPORT void								Shutdown();

		DLL_EXPORT VulkanInstance					Instance()				{ return m_instance; }
		DLL_EXPORT VulkanDebugger					Debug()					{ return m_debug; }
		DLL_EXPORT VulkanDevice						Device()				{ return m_device; }
		DLL_EXPORT VulkanSwapchain					Swapchain()				{ return m_swapChain; }
		DLL_EXPORT VulkanCommandBuffer				CommandBuffer()			{ return m_commandBuffer; }

		DLL_EXPORT [[nodiscard]] const void*		WindowHandle()			const { return m_windowHandle; }
		DLL_EXPORT [[nodiscard]] uint32				ApiVersion()			const { return m_instance.ApiVersion(); }
		DLL_EXPORT [[nodiscard]] VkInstance			InstanceHandle()		const { return m_instance.InstanceHandle(); }
		DLL_EXPORT [[nodiscard]] VkSurfaceKHR		Surface()				const { return m_instance.Surface(); }
		DLL_EXPORT [[nodiscard]] VkPhysicalDevice	PhysicalDevice()		const { return m_device.PhysicalDevice(); }
		DLL_EXPORT [[nodiscard]] VkDevice			LogicalDevice()			const { return m_device.LogicalDevice(); }

		DLL_EXPORT [[nodiscard]] ivec2				WindowResolution()		const { return m_swapChain.WindowResolution(); }
		DLL_EXPORT [[nodiscard]] uint32				SwapChainImageCount()	const { return m_swapChain.ImageIndex(); }
		DLL_EXPORT [[nodiscard]] VkExtent2D			SwapChainResolution()	const { return m_swapChain.SwapChainResolution(); }
		DLL_EXPORT [[nodiscard]] ivec2				RenderPassResolution()	const { return m_swapChain.RenderPassResolution(); }
};
extern DLL_EXPORT VulkanSystem& vulkan;
inline VulkanSystem& VulkanSystem::Get()
{
	static VulkanSystem instance;
	return instance;
}