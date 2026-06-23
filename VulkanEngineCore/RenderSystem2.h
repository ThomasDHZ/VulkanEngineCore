#pragma once
#include "Platform.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanDebugger.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

struct VulkanPipeline
{
	VkGuid						  RenderPipelineId = VkGuid();
	VkPipeline					  Pipeline = VK_NULL_HANDLE;
	VkPipelineCache				  PipelineCache = VK_NULL_HANDLE;
	VkPipelineLayout			  PipelineLayout = VK_NULL_HANDLE;
	Vector<VkDescriptorSetLayout> DescriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
	Vector<VkDescriptorSet>		  DescriptorSetList = Vector<VkDescriptorSet>();
};

class RenderSystem2
{
public:
	static RenderSystem2& Get();

private:
	RenderSystem2() = default;
	~RenderSystem2() = default;
	RenderSystem2(const RenderSystem2&) = delete;
	RenderSystem2& operator=(const RenderSystem2&) = delete;
	RenderSystem2(RenderSystem2&&) = delete;
	RenderSystem2& operator=(RenderSystem2&&) = delete;

	void DestroyPipeline(VkPipeline& pipeline);
	void DestroyPipelineLayout(VkPipelineLayout& pipelineLayout);
	void DestroyPipelineCache(VkPipelineCache& pipelineCache);

public:
	DLL_EXPORT void									  DestroyPipeline(VulkanPipeline& vulkanPipeline);
};
extern DLL_EXPORT RenderSystem2& renderSystem2;
inline RenderSystem2& RenderSystem2::Get()
{
	static RenderSystem2 instance;
	return instance;
}