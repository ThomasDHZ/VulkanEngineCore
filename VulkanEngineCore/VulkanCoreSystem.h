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

		void*				m_windowHandle = nullptr;
		VulkanInstance		m_instance;
		VulkanDebugger		m_debug;
		VulkanDevice        m_device;
		VulkanSwapchain		m_swapChain;
		VulkanCommandBuffer m_commandBuffer;

	public:
		void   RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize);
		uint32 GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
		void   Shutdown();

		VulkanInstance		 Instance()		  { return m_instance; }
		VulkanDebugger		 Debug()		  { return m_debug; }
		VulkanDevice		 Device()		  { return m_device; }
		VulkanSwapchain		 Swapchain()	  { return m_swapChain; }
		VulkanCommandBuffer	 CommandBuffer()  { return m_commandBuffer; }

		[[nodiscard]] const void*				 WindowHandle()			const { return m_windowHandle; }
		[[nodiscard]] uint32					 ApiVersion()			const { return m_instance.ApiVersion(); }
		[[nodiscard]] VkInstance				 InstanceHandle()		const { return m_instance.InstanceHandle(); }
		[[nodiscard]] VkSurfaceKHR				 Surface()				const { return m_instance.Surface(); }
		[[nodiscard]] VkPhysicalDevice			 PhysicalDevice()		const { return m_device.PhysicalDevice(); }
		[[nodiscard]] VkDevice					 LogicalDevice()		const { return m_device.LogicalDevice(); }

		[[nodiscard]] ivec2						 WindowResolution()		const { return m_swapChain.WindowResolution(); }
		[[nodiscard]] uint32					 SwapChainImageCount()  const { return m_swapChain.ImageIndex(); }
		[[nodiscard]] VkExtent2D				 SwapChainResolution()  const { return m_swapChain.SwapChainResolution(); }
		[[nodiscard]] ivec2						 RenderPassResolution() const { return m_swapChain.RenderPassResolution(); }
};
extern DLL_EXPORT VulkanSystem& vulkan;
inline VulkanSystem& VulkanSystem::Get()
{
	static VulkanSystem instance;
	return instance;
}