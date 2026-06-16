#pragma once
#include "Platform.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

class VulkanCoreSystem
{
public:
	static VulkanCoreSystem& Get();

private:
	VulkanCoreSystem() = default;
	~VulkanCoreSystem() = default;
	VulkanCoreSystem(const VulkanCoreSystem&) = delete;
	VulkanCoreSystem& operator=(const VulkanCoreSystem&) = delete;
	VulkanCoreSystem(VulkanCoreSystem&&) = delete;
	VulkanCoreSystem& operator=(VulkanCoreSystem&&) = delete;

	void*				windowHandle = nullptr;
	VulkanDevice        device;
	VulkanSwapchain     swapchain;
	VulkanCommandBuffer commandBuffer;

	VkInstance				   CreateVulkanInstance();
	Vector<const char*>		   GetRequiredInstanceExtensions();
public:

	bool Initialize(void* windowHandle, ivec2 windowSize, ivec2 renderSize);
	void BeginFrame();
	void EndFrame(VkCommandBuffer commandBuffer);
	void Shutdown();

	VulkanDevice& Device() { return device; }
	VulkanSwapchain& Swapchain() { return swapchain; }
	VulkanCommandBuffer& CommandBuffer() { return commandBuffer; }
};
extern DLL_EXPORT VulkanCoreSystem& vulkanCoreSystem;
inline VulkanCoreSystem& VulkanCoreSystem::Get()
{
	static VulkanCoreSystem instance;
	return instance;
}