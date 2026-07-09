#pragma once
#include "Platform.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"

struct PushConstantUpdateRule
{
    String                               Variable;
    String                               SourceId;
    String                               Value;
    bool                                 ConstValue;
    bool                                 DirtyFlag = true;
};

struct VulkanSubPassLoader
{
    String                               Pipeline;
    MeshTypeEnum                         MeshType;
    std::optional<String>                ShaderPushConstant;
    Vector<PushConstantUpdateRule>       PushConstantUpdates;
    Vector<VkGuid>                       InputTextureList;
    Vector<VkGuid>                       OutputTextureList;
    bool                                 OffScreenRenderPass = false;
};

struct RenderPipelineLoader
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

class DLL_EXPORT VulkanPipeline
{
private:
    VkGuid                                      m_pipelineId;
    VkPipeline                                  m_pipeline = VK_NULL_HANDLE;
    VkPipelineCache                             m_pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout                            m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool							m_globalBindlessPool = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout>               m_descriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    Vector<VkDescriptorSet>                     m_descriptorSetList = Vector<VkDescriptorSet>();

    Vector<ShaderPushConstant>                  m_pushConstantList;
    Vector<VkVertexInputAttributeDescription>   m_vertexInputAttributeList;
    Vector<VkVertexInputBindingDescription>     m_vertexInputBindingList;
    Vector<ShaderDescriptorBinding>             m_descriptorBindingList;

    void                                        ShaderToPipelineBindings(Vector<VulkanShader>& pipelineShaderList);
    void                                        CreateMemoryPoolDescriptorSets(RenderPipelineLoader& renderPipelineLoader);
    void                                        CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader);
    void                                        AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader);
    void                                        UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader);
    void                                        CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader);
    void                                        CreatePipeline(RenderPipelineLoader& renderPipelineLoader);

public:
    VulkanPipeline();
    ~VulkanPipeline();

    void                                        BuildPipelines(RenderPipelineLoader& pipelineLoader);
    void                                        Destroy();

    [[nodiscard]] VkPipeline                    Pipeline()                 const;
    [[nodiscard]] VkPipelineCache               PipelineCache()            const;
    [[nodiscard]] VkPipelineLayout              PipelineLayout()           const;
    [[nodiscard]] Vector<VkDescriptorSetLayout> DescriptorSetLayoutList()  const;
    [[nodiscard]] Vector<VkDescriptorSet>       DescriptorSetList()        const;
};

