#include "VulkanSystem.h"
#include "VulkanDebugger.h"
#include "VulkanDevice.h"
#include "VulkanWindow.h"
VulkanSystem& vulkan = VulkanSystem::Get();

void VulkanSystem::VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution)
{
    vulkanWindow.Create("Game", windowResolution.x, windowResolution.y);
    
    m_usingCustomSurface = false;
    m_windowHandle = vulkanWindow.m_window;
    m_windowResolution = windowResolution;
    RendererSetUp(vulkanWindow.m_window, renderResolution);
}

void VulkanSystem::VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution)
{
    m_usingCustomSurface = true;
    m_windowHandle = windowHandle;
    m_windowResolution = windowResolution;
    RendererSetUp(m_windowHandle, renderResolution);
}

void VulkanSystem::RendererSetUp(void* windowHandle, ivec2 renderResolution)
{
    m_instance.Initialize();
    m_device.Initialize();
    m_swapChain.Initialize(renderResolution);
    m_commandBuffer.Initialize();

#if defined(__ANDROID__)
    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)nvkGetDeviceProcAddr(Device, "vkGetBufferDeviceAddress");
    if (vkGetBufferDeviceAddress == nullptr) {
        throw std::runtime_error("Failed to load vkGetBufferDeviceAddress function pointer!");
    }
#endif
}

uint32 VulkanSystem::GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32 x = 0; x < memProperties.memoryTypeCount; x++)
    {
        if ((typeFilter & (1 << x)) &&
            (memProperties.memoryTypes[x].propertyFlags & properties) == properties)
        {
            return x;
        }
    }

    return UINT32_MAX;
}

void VulkanSystem::Shutdown()
{
}

VulkanInstance			 VulkanSystem::Instance()                     { return m_instance; }
VulkanDebugger			 VulkanSystem::Debug()                        { return m_debug; }
VulkanDevice			 VulkanSystem::Device()                       { return m_device; }
VulkanSwapchain			 VulkanSystem::Swapchain()                    { return m_swapChain; }
VulkanCommandBuffer		 VulkanSystem::CommandBuffer()                { return m_commandBuffer; }

bool					 VulkanSystem::CustomSurface()		    const { return m_usingCustomSurface; }
const void*              VulkanSystem::WindowHandle()			const { return m_windowHandle; }
ivec2					 VulkanSystem::WindowResolution()		const { return m_windowResolution; }
uint32					 VulkanSystem::ApiVersion()			    const { return m_instance.ApiVersion(); }
VkInstance				 VulkanSystem::InstanceHandle()		    const { return m_instance.InstanceHandle(); }
VkSurfaceKHR			 VulkanSystem::Surface()				const { return m_instance.Surface(); }
VkPhysicalDevice		 VulkanSystem::PhysicalDevice()		    const { return m_device.PhysicalDevice(); }
VkDevice				 VulkanSystem::LogicalDevice()		    const { return m_device.LogicalDevice(); }
VkQueue                  VulkanSystem::GraphicsQueue()		    const { return m_device.GraphicsQueue(); }
VkQueue                  VulkanSystem::PresentQueue()			const { return m_device.PresentQueue(); }
VkSampleCountFlagBits    VulkanSystem::MaxSampleCount()		    const { return m_device.MaxSampleCount(); }
uint32					 VulkanSystem::SwapChainImageCount()	const { return m_swapChain.ImageIndex(); }
VkExtent2D				 VulkanSystem::SwapChainResolution()	const { return m_swapChain.SwapChainResolution(); }
ivec2					 VulkanSystem::RenderPassResolution()	const { return m_swapChain.RenderPassResolution(); }
VkCommandPool            VulkanSystem::CommandPool()			const { return m_commandBuffer.CommandPool(); }
Vector<VkCommandBuffer>  VulkanSystem::CommandBufferList()	    const { return m_commandBuffer.CommandBufferList(); }