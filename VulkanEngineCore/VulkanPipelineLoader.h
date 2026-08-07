#pragma once
#include "Platform.h"
#include "VulkanShader.h"


struct VulkanPipelineLoader
{
    VkGuid                                      PipelineId = VkGuid();
    Vector<VulkanShader>                        VulkanShaderList;
    Vector<ShaderLoader>                        ShaderLoaderList;
    Vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList;
    VkPipelineInputAssemblyStateCreateInfo      PipelineInputAssemblyStateCreateInfo = VkPipelineInputAssemblyStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo      PipelineRasterizationStateCreateInfo = VkPipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo        PipelineMultisampleStateCreateInfo = VkPipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo       PipelineDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo         PipelineColorBlendStateCreateInfoModel = VkPipelineColorBlendStateCreateInfo();
    bool                                        UseDynamicColorWrite = false;
};

struct VulkanPipelinePackageLoader
{
    VkGuid                              PipelinePackageId = VkGuid();
    VkGuid                              RenderPassId = VkGuid();
   // VkGuid                              LevelId = VkGuid();
    uint32                              SubPassId = UINT32_MAX;
    uint32                              BindlessDescriptorSetIndex = UINT32_MAX;
    ivec2                               RenderPassResolution = ivec2();
    VkRenderPass                        RenderPass = VK_NULL_HANDLE;
    VkDescriptorPool					GlobalBindlessPool = VK_NULL_HANDLE;
    VkDescriptorSet						GlobalBindlessDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout				GlobalBindlessDescriptorSetLayout = VK_NULL_HANDLE;
    Vector<VkDescriptorImageInfo>       RenderPassInputTextures;
    Vector<VkViewport>                  ViewportList;
    Vector<VkRect2D>                    ScissorList;

    VulkanPipelineLoader                DefaultPipeline;
    std::optional<VulkanPipelineLoader> DepthPipeline;
    std::optional<VulkanPipelineLoader> WireFramePipeline;
    std::optional<VulkanPipelineLoader> ReflectionPipeline;
    std::optional<VulkanPipelineLoader> CollisionPipeline;

    bool                                UseGlobalBindlessSet = false;
    bool                                UseCubeMapMultiview = false;
};
