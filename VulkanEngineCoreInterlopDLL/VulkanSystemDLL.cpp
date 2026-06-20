#include "VulkanSystemDLL.h"

void RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize)
{
    return vulkan.RendererSetUp(windowHandle, windowSize, renderSize);
}

uint32 GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    return vulkan.GetMemoryType(physicalDevice, typeFilter, properties);
}

void Shutdown()
{
    vulkan.Shutdown();
}
