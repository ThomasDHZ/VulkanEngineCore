#include "VulkanSystem.h"
#include "VulkanDebugger.h"
#include "VulkanDevice.h"

VulkanSystem& vulkan = VulkanSystem::Get();

void VulkanSystem::RendererSetUp(void* windowHandle, ivec2 windowResolution, ivec2 defaultRenderPassResolution)
{
    m_windowHandle = windowHandle;
    m_windowResolution = windowResolution;
    m_instance.Initialize();
    m_device.Initialize();
    m_swapChain.Initialize(defaultRenderPassResolution);
    m_commandBuffer.Initialize();
    //bufferSystem.vmaAllocator = SetUpVmaAllocation();


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
