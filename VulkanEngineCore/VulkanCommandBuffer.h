#pragma once
#include "Platform.h"

class VulkanCommandBuffer
{
	friend class VulkanDevice;
private:
	VkCommandPool						   m_CommandPool = VK_NULL_HANDLE;
	Vector<VkCommandBuffer>				   m_CommandBufferList = Vector<VkCommandBuffer>();

	void								   SetUpCommandBuffers();
	VkCommandPool						   SetUpCommandPool();

public:
	VulkanCommandBuffer();
	~VulkanCommandBuffer();

	void								   Initialize();
	VkCommandBuffer						   BeginSingleUseCommand();
	void								   EndSingleUseCommand(VkCommandBuffer commandBuffer);

	[[nodiscard]] VkCommandPool            CommandPool()	   const { return m_CommandPool; }
	[[nodiscard]] Vector<VkCommandBuffer>  CommandBufferList() const { return m_CommandBufferList; }
};

