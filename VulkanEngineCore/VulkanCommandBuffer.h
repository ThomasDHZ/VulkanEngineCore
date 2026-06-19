#pragma once
#include "Platform.h"

class VulkanCommandBuffer
{
	friend class VulkanDevice;
private:
	void								   SetUpCommandBuffers();
	VkCommandPool						   SetUpCommandPool();

public:
	VkCommandPool						   CommandPool = VK_NULL_HANDLE;
	Vector<VkCommandBuffer>				   CommandBuffers = Vector<VkCommandBuffer>();

	void								   Initialize();
	VkCommandBuffer						   BeginSingleUseCommand();
	void								   EndSingleUseCommand(VkCommandBuffer commandBuffer);
};

