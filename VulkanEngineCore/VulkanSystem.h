#pragma once
#include "DLL.h"
#include <Platform.h>
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

	void*								  m_windowHandle = nullptr;
	ivec2								  m_windowResolution;
	VulkanInstance						  m_instance;
	VulkanDebugger						  m_debug;
	VulkanDevice						  m_device;
	VulkanSwapchain						  m_swapChain;
	VulkanCommandBuffer					  m_commandBuffer;
	bool								  m_framebufferResized = false;
	bool								  m_usingCustomSurface = false;

	void								  RendererSetUp(void* windowHandle, ivec2 renderResolution);

public:

	CORE_DLL_EXPORT void VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution);
	CORE_DLL_EXPORT void VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution);
	CORE_DLL_EXPORT uint32 GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
	CORE_DLL_EXPORT VkCommandBuffer StartFrame();
	CORE_DLL_EXPORT void EndFrame(VkCommandBuffer& commandBuffer);
	CORE_DLL_EXPORT void Destroy();
	CORE_DLL_EXPORT void SetCustomFrameBufferSize(ivec2 size);
	CORE_DLL_EXPORT void TriggerFrameBufferResized();

	 bool WasFramebufferResized() const { return m_framebufferResized; }
	 void ResetFramebufferResized() { m_framebufferResized = false; }

    VulkanSwapchain&      Swapchain()      { return m_swapChain; }
    VulkanCommandBuffer&  CommandBuffer()  { return m_commandBuffer; }
    VulkanDebugger&       Debug()          { return m_debug; }
    VulkanInstance&       Instance()       { return m_instance; }
    VulkanDevice&         Device()         { return m_device; }

	 CORE_DLL_EXPORT [[nodiscard]] bool					  CustomSurface()			const;
	 CORE_DLL_EXPORT [[nodiscard]] const void*			  WindowHandle()			const;
	 CORE_DLL_EXPORT [[nodiscard]] ivec2					  WindowResolution()		const;
	 CORE_DLL_EXPORT [[nodiscard]] uint32				  ApiVersion()				const;
	 CORE_DLL_EXPORT [[nodiscard]] VkInstance			  InstanceHandle()			const;
	 CORE_DLL_EXPORT [[nodiscard]] VkSurfaceKHR			  Surface()					const;
	 CORE_DLL_EXPORT [[nodiscard]] VkPhysicalDevice		  PhysicalDevice()			const;
	CORE_DLL_EXPORT [[nodiscard]] VkDevice				  LogicalDevice()			const;
	CORE_DLL_EXPORT [[nodiscard]] VkQueue                 GraphicsQueue()			const;
	CORE_DLL_EXPORT [[nodiscard]] VkQueue                 PresentQueue()			const;
	CORE_DLL_EXPORT [[nodiscard]] VkSampleCountFlagBits   MaxSampleCount()			const;
	CORE_DLL_EXPORT [[nodiscard]] uint32				  SwapChainImageCount()		const;
	CORE_DLL_EXPORT [[nodiscard]] VkExtent2D			  SwapChainResolution()		const;
	CORE_DLL_EXPORT [[nodiscard]] ivec2					  RenderPassResolution()	const;
	CORE_DLL_EXPORT [[nodiscard]] VkCommandPool           CommandPool()				const;
	CORE_DLL_EXPORT [[nodiscard]] Vector<VkCommandBuffer> CommandBufferList()		const;
};
CORE_DLL_EXPORT extern VulkanSystem& vulkan;
inline VulkanSystem& VulkanSystem::Get()
{
	static VulkanSystem instance;
	return instance;
}