#pragma once

#include "Platform.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanPipelineLoader.h"

class VulkanPipeline
{
private:
    VkGuid                                      m_pipelineId;
    Vector<ShaderPushConstant>                  m_pushConstantList;
    VkPipeline                                  m_pipeline = VK_NULL_HANDLE;
    VkPipelineCache                             m_pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout                            m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool							m_globalBindlessPool = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout>               m_descriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    Vector<VkDescriptorSet>                     m_descriptorSetList = Vector<VkDescriptorSet>();
    Vector<VulkanShader>                        m_shaderList;

    Vector<VkVertexInputAttributeDescription>   m_vertexInputAttributeList;
    Vector<VkVertexInputBindingDescription>     m_vertexInputBindingList;
    Vector<ShaderDescriptorBinding>             m_descriptorBindingList;
    VkPolygonMode                               m_polygonMode = VK_POLYGON_MODE_FILL;

    void                                        ShaderToPipelineBindings(const VulkanPipelinePackageLoader& pipelinePackage, Vector<VulkanShader>& pipelineShaderList);
    void                                        CreateMemoryPoolDescriptorSets(const VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);
    void                                        CreatePipelineDescriptorSetLayout(const VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);
    void                                        AllocatePipelineDescriptorSets(const VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);
    void                                        UpdatePipelineDescriptorSets(const VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);
    void                                        CreatePipelineLayout(const VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);
    void                                        CreatePipeline(VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);

public:
    VulkanPipeline();
    ~VulkanPipeline();

    VkGuid                                      BuildPipelines(VulkanPipelinePackageLoader& pipelinePackage, VulkanPipelineLoader& pipelineLoader);
    void                                        Destroy();

    [[nodiscard]] VkGuid                        PipelineId()               const;
    [[nodiscard]] VkPipeline                    Pipeline()                 const;
    [[nodiscard]] VkPipelineCache               PipelineCache()            const;
    [[nodiscard]] VkPipelineLayout              PipelineLayout()           const;
    [[nodiscard]] Vector<VkDescriptorSetLayout> DescriptorSetLayoutList()  const;
    [[nodiscard]] Vector<VkDescriptorSet>       DescriptorSetList()        const;
    [[nodiscard]] Vector<ShaderPushConstant>    ShaderPushConstantList()   const;
};

