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

struct RenderPassAttachmentTexture
{
    VkGuid                               RenderedTextureId = VkGuid();
    uint32                               MipMapCount = UINT32_MAX;
    TextureTypeEnum                      TextureType = TextureTypeEnum::kTextureType_Undefined;
    TextureUsageTypeEnum                 TextureUsageType = kUsageType_Undefined;
    Vector<RenderAttachmentTypeEnum>     RenderAttachmentTypes = Vector<RenderAttachmentTypeEnum>();
    VkFormat                             Format = VK_FORMAT_R8G8B8A8_UNORM;
    VkAttachmentLoadOp                   LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp                  StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkSamplerCreateInfo                  SamplerCreateInfo = VkSamplerCreateInfo();
    VkImageLayout                        FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    bool                                 UseMipMaps = false;
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

struct RenderPassLoader
{
    VkGuid                               RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(INT32_MAX, INT32_MAX);
    Vector<Vector<VulkanSubPassLoader>>  SubPassList;
    Vector<String>                       RenderPipelineList;
    Vector<RenderPassAttachmentTexture>  RenderAttachmentList;
    Vector<VkSubpassDependency>          SubpassDependencyList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;
};

struct RenderPipelineLoader
{
    VkGuid PipelineId = VkGuid();
    VkGuid RenderPassId = VkGuid();
    VkGuid LevelId = VkGuid();
    uint32 SubPassId = UINT32_MAX;
    ivec2 RenderPassResolution = ivec2();
    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkDescriptorPool										 GlobalBindlessPool = VK_NULL_HANDLE;
    Vector<VulkanShader>                                     VulkanShaderList;
    Vector<VkDescriptorImageInfo>                            RenderPassInputTextures;
    Vector<VkViewport> ViewportList;
    Vector<VkRect2D> ScissorList;
    Vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList;
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo = VkPipelineInputAssemblyStateCreateInfo();
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo = VkPipelineRasterizationStateCreateInfo();
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo = VkPipelineMultisampleStateCreateInfo();
    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo();
    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel = VkPipelineColorBlendStateCreateInfo();
    bool UseDynamicColorWrite = false;
    bool UseCubeMapMultiview = false;
};

class DLL_EXPORT VulkanPipeline
{
private:
    VkGuid                                      m_pipelineId;
    VkPipeline                                  m_pipeline = VK_NULL_HANDLE;
    VkPipelineCache                             m_pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout                            m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet                             m_globalBindlessDescriptorSet = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout>               m_descriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    Vector<VkDescriptorSet>                     m_descriptorSetList = Vector<VkDescriptorSet>();

    ShaderPushConstant                          m_pushConstant;
    Vector<VkVertexInputAttributeDescription>   m_vertexInputAttributeList;
    Vector<VkVertexInputBindingDescription>     m_vertexInputBindingList;
    Vector<ShaderDescriptorBinding>             m_descriptorBindingList;

    uint32                                      m_globalBindlessSetIndex = UINT32_MAX;
    bool                                        m_useGlobalBindlessSet = false;

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
    [[nodiscard]] VkPipeline                    Pipeline()                 const;
    [[nodiscard]] VkPipelineCache               PipelineCache()            const;
    [[nodiscard]] VkPipelineLayout              PipelineLayout()           const;
    [[nodiscard]] Vector<VkDescriptorSetLayout> DescriptorSetLayoutList()  const;
    [[nodiscard]] Vector<VkDescriptorSet>       DescriptorSetList()        const;
};

