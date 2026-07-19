#pragma once
#include <Platform.h>

class DLL_EXPORT VulkanCommandBuffer
{
	friend class VulkanDevice;
private:
	VkCommandPool								 m_CommandPool = VK_NULL_HANDLE;
	Vector<VkCommandBuffer>						 m_CommandBufferList;

	void										 SetUpCommandBuffers();
	void										 SetUpCommandPool();

public:
	VulkanCommandBuffer();
	~VulkanCommandBuffer();

	void										 Initialize();
	[[nodiscard]] VkCommandBuffer				 GetCurrentCommandBuffer() const;
	VkCommandBuffer								 BeginSingleUseCommand();
	void										 EndSingleUseCommand(VkCommandBuffer commandBuffer);
	void										 Destroy();

	[[nodiscard]] VkCommandPool					 CommandPool()		 const;
	[[nodiscard]] const Vector<VkCommandBuffer>& CommandBufferList() const;
};