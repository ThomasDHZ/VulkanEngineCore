#pragma once
#include <Platform.h>
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanDebugger.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

class DLL_EXPORT VulkanSystem2
{
public:
	static VulkanSystem2& Get();

private:
	VulkanSystem2() = default;
	~VulkanSystem2() = default;
	VulkanSystem2(const VulkanSystem2&) = delete;
	VulkanSystem2& operator=(const VulkanSystem2&) = delete;
	VulkanSystem2(VulkanSystem2&&) = delete;
	VulkanSystem2& operator=(VulkanSystem2&&) = delete;

	void*								  m_windowHandle = nullptr;
	ivec2								  m_windowResolution;
	VulkanInstance						  m_instance;
	VulkanDebugger						  m_debug;
	VulkanDevice						  m_device;
	VulkanSwapchain						  m_swapChain;
	VulkanCommandBuffer					  m_commandBuffer;
	bool								  m_usingCustomSurface = false;

	void								  RendererSetUp(void* windowHandle, ivec2 renderResolution);

public:
	void								  VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution);
	void								  VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution);
	uint32								  GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
	void								  Shutdown();

	VulkanInstance						  Instance();
	VulkanDebugger						  Debug();
	VulkanDevice						  Device();
	VulkanSwapchain						  Swapchain();
	VulkanCommandBuffer					  CommandBuffer();

	[[nodiscard]] bool					  CustomSurface()			const;
	[[nodiscard]] const void*			  WindowHandle()			const;
	[[nodiscard]] ivec2					  WindowResolution()		const;
	[[nodiscard]] uint32				  ApiVersion()				const;
	[[nodiscard]] VkInstance			  InstanceHandle()			const;
	[[nodiscard]] VkSurfaceKHR			  Surface()					const;
	[[nodiscard]] VkPhysicalDevice		  PhysicalDevice()			const;
	[[nodiscard]] VkDevice				  LogicalDevice()			const;
	[[nodiscard]] VkQueue                 GraphicsQueue()			const;
	[[nodiscard]] VkQueue                 PresentQueue()			const;
	[[nodiscard]] VkSampleCountFlagBits   MaxSampleCount()			const;
	[[nodiscard]] uint32				  SwapChainImageCount()		const;
	[[nodiscard]] VkExtent2D			  SwapChainResolution()		const;
	[[nodiscard]] ivec2					  RenderPassResolution()	const;
	[[nodiscard]] VkCommandPool           CommandPool()				const;
	[[nodiscard]] Vector<VkCommandBuffer> CommandBufferList()		const;

};
extern DLL_EXPORT VulkanSystem2& vulkan;
inline VulkanSystem2& VulkanSystem2::Get()
{
	static VulkanSystem2 instance;
	return instance;
}