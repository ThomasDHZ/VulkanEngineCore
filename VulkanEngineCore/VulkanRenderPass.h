#pragma once

#include "Platform.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLoader.h"
#include "VulkanTexture.h"

enum MeshTypeEnum
{
    kMesh_None,
    kMesh_StaticMesh,
    kMesh_InstanceMesh,
    kMesh_Skinned,
    kMesh_Procedural,
    kMesh_FrameBuffer,
    kMesh_Undefined
};

struct MeshDrawMessage
{
    uint32		   MeshId = UINT32_MAX;
    uint32	       Drawlayer = UINT32_MAX;
    uint32         VertexBufferBinding = 0;
    uint32		   VertexCount = 0;
    uint32		   IndexCount = 0;
    uint32		   InstanceCount = 1;
    uint32         FirstVertex = 0;
    uint32	       FirstIndex = 0;
    uint32	       StartInstanceIndex = 0;
    VkDeviceSize   VertexOffset = 0;
    VkDeviceSize   InstanceOffset = 0;
    VkBuffer	   VertexBuffer = VK_NULL_HANDLE;
    VkBuffer	   IndexBuffer = VK_NULL_HANDLE;
    VkBuffer       InstanceBuffer = VK_NULL_HANDLE;
};

struct VulkanSubPass
{
    VkGuid                               RenderPassGuid;
    VkGuid                               PipelinePackageId;
    MeshTypeEnum                         MeshType;
    std::optional<String>                ShaderPushConstant;
    Vector<VkGuid>                       InputTextureList;
    Vector<VkGuid>                       OutputTextureList;
    bool                                 OffScreenFrameBuffer = false;
};

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
    VkGuid                               PipelinePackageId;
    MeshTypeEnum                         MeshType;
    std::optional<String>                ShaderPushConstant;
    Vector<PushConstantUpdateRule>       PushConstantUpdates;
    Vector<VkGuid>                       InputTextureList;
    Vector<VkGuid>                       OutputTextureList;
    bool                                 OffScreenRenderPass = false;
};

struct VulkanPipelinePackage
{
    VkGuid                                  PipelinePackageId;
    UnorderedMap<PipelineTypeEnum, VkGuid>  PipelineMap;
};

struct RenderPassLoader
{
    VkGuid                               RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(0);
    Vector<RenderPassAttachmentLoader>   AttachmentList;
    Vector<VkSubpassDependency>          SubpassDependencyList;
    Vector<String>                       PipelineList;
    Vector<VulkanPipelinePackage>        PipelinePackageList;
    Vector<Vector<VulkanSubPassLoader>>  SubPassList;
    Vector<ShaderLoader>                 ShaderList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;

    bool                                 UseDefaultRenderResolution = true;
    bool                                 UseGlobalBindlessSet = false;
    bool                                 UseVkMultiview = false;
    bool                                 RenderAsCubemap = false;
};

struct VulkanRenderPass
{
public:
    static constexpr uint32                                 MAX_FRAMES_IN_FLIGHT = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;

private:

    VkGuid                                                  m_renderPassId = VkGuid();
    ivec2                                                   m_renderPassResolution = ivec2(0);
    VkRenderPass                                            m_renderPass = VK_NULL_HANDLE;
    Vector<VkGuid>                                          m_pipelineList;
    Vector<VkGuid>                                          m_attachmentIdList;
    std::array<Vector<VkFramebuffer>, MAX_FRAMES_IN_FLIGHT> m_frameBufferList;
    std::array<Vector<VulkanTexture>, MAX_FRAMES_IN_FLIGHT> m_attachmentList;
    Vector<RenderPassAttachmentReloader>                    m_renderPassReloaderList;
    Vector<Vector<VulkanSubPass>>                           m_subPassList;
    Vector<VkClearValue>                                    m_clearValueList;
    VkSampleCountFlagBits                                   m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkGuid                                                  m_currentBoundPipeline;
    bool                                                    m_useVkMultiview = false;
    bool                                                    m_renderAsCubemap = false;
    bool                                                    m_useDefaultRenderResolution = true;

    void                                                    BuildRenderPass(RenderPassLoader& renderPassLoader);
    VulkanSubPass                                           BuildSubpasses(VulkanSubPassLoader& subPassLoader);
    Vector<VkAttachmentDescription>                         BuildAttachmentDescriptors(RenderPassLoader& renderPassLoader);
    void                                                    BuildAttachments(Vector<RenderPassAttachmentLoader>& attachmentTextureList);
    void                                                    BuildFrameBuffer();
    void                                                    TransitionRenderPassAttachmentsToFinalLayout();
    bool                                                    RenderPipelineExists(const VkGuid& pipelineId);

public:
    VulkanRenderPass();
    ~VulkanRenderPass();

    void                                                    LoadRenderPass(RenderPassLoader& renderPassLoader);
    void                                                    AddRenderPipeline(const VkGuid& pipelineId);
    void                                                    RebuildRenderPass();
    VulkanTexture                                           FindRenderPassAttachment(const VkGuid& attachmentId);
    bool                                                    RenderPassAttachmentExists(const VkGuid& attachmentId);
    void                                                    BeginRenderPass(VkCommandBuffer& commandBuffer, uint mipLevel = 0);
    void                                                    NextSubpass(VkCommandBuffer& commandBuffer);
    void                                                    BindViewPort(VkCommandBuffer& commandBuffer, uint drawMipLevel = 0);
    void                                                    BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet);
    void                                                    DrawMesh(VkCommandBuffer cmd, MeshDrawMessage& mesh);
    void                                                    EndRenderPass(VkCommandBuffer& commandBuffer);
    void                                                    Destroy();

    [[nodiscard]] VkGuid                                    RenderPassId()               const noexcept;
    [[nodiscard]] VkRenderPass                              RenderPassHandle()           const noexcept;
    [[nodiscard]] ivec2                                     RenderPassResolution()       const noexcept;
    [[nodiscard]] Vector<VulkanTexture>                     AttachmentList()             const noexcept;
    [[nodiscard]] Vector<VkGuid>                            PipelineList()               const noexcept;
    [[nodiscard]] Vector<Vector<VulkanSubPass>>             SubPassList()                const noexcept;
    [[nodiscard]] VkSampleCountFlagBits                     SampleCount()                const noexcept;
    [[nodiscard]] bool                                      RenderAsCubemap()            const noexcept;
};