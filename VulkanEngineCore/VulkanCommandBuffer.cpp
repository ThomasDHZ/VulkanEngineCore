#include "VulkanCommandBuffer.h"
#include "VulkanCoreSystem.h"
#include "VulkanSwapchain.h" 
#include "VulkanDevice.h"

void VulkanCommandBuffer::SetUpCommandBuffers()
{
    CommandBuffers = Vector<VkCommandBuffer>(vulkanCoreSystem.Swapchain().SwapChainImageCount, VK_NULL_HANDLE);
    for (size_t x = 0; x < vulkanCoreSystem.Swapchain().SwapChainImageCount; x++)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32>(vulkanCoreSystem.Swapchain().SwapChainImageCount)
        };
        vkAllocateCommandBuffers(vulkanCoreSystem.Device().LogicalDevice, &commandBufferAllocateInfo, &CommandBuffers[x]);
    }
}

VkCommandPool VulkanCommandBuffer::SetUpCommandPool(VkDevice device, uint32 graphicsFamily)
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo CommandPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphicsFamily
    };
    VULKAN_THROW_IF_FAIL(vkCreateCommandPool(device, &CommandPoolCreateInfo, NULL, &commandPool));
    return commandPool;
}

VkCommandBuffer VulkanCommandBuffer::BeginSingleUseCommand()
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(vulkanCoreSystem.Device().LogicalDevice, &allocInfo, &commandBuffer));

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

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkanCoreSystem.Device().GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VULKAN_THROW_IF_FAIL(vkQueueWaitIdle(vulkanCoreSystem.Device().GraphicsQueue));

    vkFreeCommandBuffers(vulkanCoreSystem.Device().LogicalDevice, CommandPool, 1, &commandBuffer);
}
