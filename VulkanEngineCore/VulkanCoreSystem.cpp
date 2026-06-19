#include "VulkanCoreSystem.h"
#include "VulkanDebugger.h"
#include "VulkanDevice.h"

VulkanSystem& vulkan = VulkanSystem::Get();

void VulkanSystem::RendererSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize)
{
    m_windowHandle = windowHandle;
    m_instance.Initialize();
    m_device.Initialize();
    m_swapChain.Initialize();
    m_commandBuffer.Initialize();
    //bufferSystem.vmaAllocator = SetUpVmaAllocation();
    //vulkanSystem.MaxSampleCount = GetMaxSampleCount(vulkanSystem.PhysicalDevice);


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

    for (uint32_t x = 0; x < memProperties.memoryTypeCount; x++)
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
