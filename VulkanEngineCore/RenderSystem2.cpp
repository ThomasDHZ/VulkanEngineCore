#include "RenderSystem2.h"
#include "VulkanSystem2.h"

RenderSystem2& renderSystem2 = RenderSystem2::Get();

void RenderSystem2::DestroyPipeline(VulkanPipeline& pipeline)
{
    pipeline.RenderPipelineId = VkGuid();
    DestroyPipeline(pipeline.Pipeline);
    DestroyPipelineLayout(pipeline.PipelineLayout);
    DestroyPipelineCache(pipeline.PipelineCache);
}

void RenderSystem2::DestroyPipeline(VkPipeline& pipeline)
{
    if (pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(vulkan.LogicalDevice(), pipeline, NULL);
        pipeline = VK_NULL_HANDLE;
    }
}

void RenderSystem2::DestroyPipelineLayout(VkPipelineLayout& pipelineLayout)
{
    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(vulkan.LogicalDevice(), pipelineLayout, NULL);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void RenderSystem2::DestroyPipelineCache(VkPipelineCache& pipelineCache)
{
    if (pipelineCache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(vulkan.LogicalDevice(), pipelineCache, NULL);
        pipelineCache = VK_NULL_HANDLE;
    }
}
