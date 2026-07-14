#pragma once
#include "Platform.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanPipelineLoader.h"

class DLL_EXPORT VulkanPipeline
{
private:
    VkPipeline                                  m_pipeline = VK_NULL_HANDLE;
    VkPipelineCache                             m_pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout                            m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool							m_globalBindlessPool = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout>               m_descriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    Vector<VkDescriptorSet>                     m_descriptorSetList = Vector<VkDescriptorSet>();

    Vector<VkVertexInputAttributeDescription>   m_vertexInputAttributeList;
    Vector<VkVertexInputBindingDescription>     m_vertexInputBindingList;
    Vector<ShaderDescriptorBinding>             m_descriptorBindingList;

    void                                        ShaderToPipelineBindings(Vector<VulkanShader>& pipelineShaderList);
    void                                        CreateMemoryPoolDescriptorSets(VulkanPipelineLoader& pipelineLoader);
    void                                        CreatePipelineDescriptorSetLayout(VulkanPipelineLoader& pipelineLoader);
    void                                        AllocatePipelineDescriptorSets(VulkanPipelineLoader& pipelineLoader);
    void                                        UpdatePipelineDescriptorSets(VulkanPipelineLoader& pipelineLoader);
    void                                        CreatePipelineLayout(VulkanPipelineLoader& pipelineLoader);
    void                                        CreatePipeline(VulkanPipelineLoader& pipelineLoader);

public:
    VulkanPipeline();
    ~VulkanPipeline();

    VkGuid                                      m_pipelineId;
    Vector<ShaderPushConstant>                  m_pushConstantList;
    void                                        BuildPipelines(VulkanPipelineLoader& pipelineLoader);
    void                                        Destroy();

    [[nodiscard]] VkPipeline                    Pipeline()                 const;
    [[nodiscard]] VkPipelineCache               PipelineCache()            const;
    [[nodiscard]] VkPipelineLayout              PipelineLayout()           const;
    [[nodiscard]] Vector<VkDescriptorSetLayout> DescriptorSetLayoutList()  const;
    [[nodiscard]] Vector<VkDescriptorSet>       DescriptorSetList()        const;
};

