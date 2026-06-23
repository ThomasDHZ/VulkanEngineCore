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
    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vkGetDeviceProcAddr(Device, "vkGetBufferDeviceAddress");
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
