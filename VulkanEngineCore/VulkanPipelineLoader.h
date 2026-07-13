#pragma once
#include "Platform.h"
#include "VulkanShader.h"

struct VulkanPipelineLoader
{
    VkGuid                                      PipelineId = VkGuid();
    VkGuid                                      RenderPassId = VkGuid();
    VkGuid                                      LevelId = VkGuid();
    uint32                                      SubPassId = UINT32_MAX;
    uint32                                      BindlessDescriptorSetIndex = UINT32_MAX;
    ivec2                                       RenderPassResolution = ivec2();
    VkRenderPass                                RenderPass = VK_NULL_HANDLE;
    VkDescriptorPool							GlobalBindlessPool = VK_NULL_HANDLE;
    VkDescriptorSet								GlobalBindlessDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout						GlobalBindlessDescriptorSetLayout = VK_NULL_HANDLE;
    Vector<VulkanShader>                        VulkanShaderList;
    Vector<VkDescriptorImageInfo>               RenderPassInputTextures;
    Vector<VkViewport>                          ViewportList;
    Vector<VkRect2D>                            ScissorList;
    Vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList;
    VkPipelineInputAssemblyStateCreateInfo      PipelineInputAssemblyStateCreateInfo = VkPipelineInputAssemblyStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo      PipelineRasterizationStateCreateInfo = VkPipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo        PipelineMultisampleStateCreateInfo = VkPipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo       PipelineDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo         PipelineColorBlendStateCreateInfoModel = VkPipelineColorBlendStateCreateInfo();
    bool                                        UseGlobalBindlessSet = false;
    bool                                        UseDynamicColorWrite = false;
    bool                                        UseCubeMapMultiview = false;
};

