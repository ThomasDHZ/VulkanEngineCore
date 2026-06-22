#include "VulkanCommandBuffer.h"
#include "VulkanSystem.h"
#include "VulkanSwapchain.h" 
#include "VulkanDevice.h"

VulkanCommandBuffer::VulkanCommandBuffer()
{

}

VulkanCommandBuffer::~VulkanCommandBuffer()
{

}

void VulkanCommandBuffer::Initialize()
{
    SetUpCommandPool();
    SetUpCommandBuffers();
}

void VulkanCommandBuffer::SetUpCommandBuffers()
{
    m_CommandBufferList = Vector<VkCommandBuffer>(vulkan.Swapchain().SwapChainImageCount(), VK_NULL_HANDLE);
    for (size_t x = 0; x < vulkan.Swapchain().SwapChainImageCount(); x++)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32>(vulkan.Swapchain().SwapChainImageCount())
        };
        vkAllocateCommandBuffers(vulkan.Device().LogicalDevice(), &commandBufferAllocateInfo, &m_CommandBufferList[x]);
    }
}

VkCommandPool VulkanCommandBuffer::SetUpCommandPool()
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = vulkan.Device().GraphicsFamily()
    };
    VULKAN_THROW_IF_FAIL(vkCreateCommandPool(vulkan.LogicalDevice(), &CommandPoolCreateInfo, NULL, &m_CommandPool));
}

VkCommandBuffer VulkanCommandBuffer::BeginSingleUseCommand()
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(vulkan.LogicalDevice(), &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

void VulkanCommandBuffer::EndSingleUseCommand(VkCommandBuffer commandBuffer)
{
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = VkSubmitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkan.Device().GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_THROW_IF_FAIL(vkQueueWaitIdle(vulkan.Device().GraphicsQueue()));
    vkFreeCommandBuffers(vulkan.Device().LogicalDevice(), m_CommandPool, 1, &commandBuffer);
}
