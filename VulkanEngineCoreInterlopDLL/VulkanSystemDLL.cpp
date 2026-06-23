#include "VulkanSystemDLL.h"

void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
{
    vulkan.Debug().CreateLogMessageCallback(callback);
}

void VulkanSystem_VulkanSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize)
{
    vulkan.VulkanSetUp(windowHandle, windowSize, renderSize);
}

uint32 VulkanSystem_GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    return vulkan.GetMemoryType(physicalDevice, typeFilter, properties);
}

void VulkanSystem_Shutdown()
{
    vulkan.Shutdown();
}
