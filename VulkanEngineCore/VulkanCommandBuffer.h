#pragma once
#include "Platform.h"

class VulkanCommandBuffer
{
private:
	void								   SetUpCommandBuffers();
	VkCommandPool						   SetUpCommandPool(VkDevice device, uint32 graphicsFamily);

public:
	VkCommandPool						   CommandPool = VK_NULL_HANDLE;
	Vector<VkCommandBuffer>				   CommandBuffers = Vector<VkCommandBuffer>();

	VkCommandBuffer						   BeginSingleUseCommand();
	void								   EndSingleUseCommand(VkCommandBuffer commandBuffer);
};

