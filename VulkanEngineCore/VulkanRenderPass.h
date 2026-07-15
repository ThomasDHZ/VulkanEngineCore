#pragma once
#include "Platform.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLoader.h"
#include "VulkanTexture.h"

enum MeshTypeEnum
{
    kMesh_None,
    kMesh_SpriteMesh,
    kMesh_LevelMesh,
    kMesh_SkyBoxMesh,
    kMesh_LineMesh,
    kMesh_LevelEditorIconMesh,
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
    VkGuid                               PipelineGuid;
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
    ivec2                                RenderPassResolution = ivec2(INT32_MAX);
    Vector<RenderPassAttachmentLoader>   AttachmentList;
    Vector<VkSubpassDependency>          SubpassDependencyList;
    Vector<VulkanPipelineLoader>         PipelineList;
    Vector<Vector<VulkanSubPassLoader>>  SubPassList;
    Vector<VulkanShader>                 ShaderList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseGlobalBindlessSet = false;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;
};

//class DLL_EXPORT VulkanRenderPass
//{
//private:
//    void                            BuildRenderPass(RenderPassLoader& renderPassLoader);
//    void                            BuildPipelines(RenderPassLoader& renderPassLoader);
//    VulkanSubPass                   BuildSubpasses(RenderPassLoader& renderPassLoader, VulkanSubPassLoader& subPassLoader);
//    void                            BuildAttachmentDescriptors(RenderPassLoader& renderPassLoader);
//    void                            BuildAttachments(Vector<RenderPassAttachmentLoader>& attachmentTextureList);
//    void                            BuildFrameBuffer(RenderPassLoader& renderPassLoader);
//public:
//     
//    VkGuid                          m_renderPassId = VkGuid();
//    ivec2                           m_renderPassResolution = ivec2(INT32_MAX);
//    VkRenderPass                    m_renderPass = VK_NULL_HANDLE;
//    Vector<VulkanPipeline>          m_pipelineList;
//    Vector<VulkanSubPass>           m_subPassList;
//    Vector<VkFramebuffer>           m_frameBufferList;
//    Vector<VulkanTexture>           m_attachmentList;
//    Vector<VkAttachmentDescription> m_attachmentDescriptionList;
//    VulkanTexture                   m_depthAttachment;
//    Vector<VkClearValue>            m_clearValueList;
//    VkSampleCountFlagBits           m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
//    bool                            m_useCubeMapMultiView = false;
//    bool                            m_isCubeMapRenderPass = false;
//
//
//    VulkanRenderPass();
//    ~VulkanRenderPass();
//
//    void                            LoadRenderPass(RenderPassLoader& renderPassLoader);
//};

struct DLL_EXPORT VulkanRenderPass
{
private:
    const VulkanPipeline* FindRenderPipeline(const VkGuid& pipelineId);

public:

    VkGuid                               RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(INT32_MAX, INT32_MAX);
    VkRenderPass                         RenderPass = VK_NULL_HANDLE;
    Vector<VulkanPipeline>               PipelineList;
    Vector<VkFramebuffer>                FrameBufferList;
    Vector<VulkanTexture>                AttachmentList;
    Vector<VkAttachmentDescription>      AttachmentDescriptionList;
    Vector<Vector<VulkanSubPass>>        SubPassList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    VulkanTexture                        m_depthAttachment;
    Vector<VulkanTexture>                m_frameBufferAttachments;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;

        VulkanRenderPass();
        ~VulkanRenderPass();
    
        void                            LoadRenderPass(RenderPassLoader& renderPassLoader);
        void                            BuildRenderPass(RenderPassLoader& renderPassLoader);
        void                            BuildPipeline(VulkanPipelineLoader& pipelineLoader, bool useGlobalBindlessSet);
        VulkanSubPass                   BuildSubpasses(RenderPassLoader& renderPassLoader, VulkanSubPassLoader& subPassLoader);
        void                            BuildAttachmentDescriptors(RenderPassLoader& renderPassLoader);
        void                            BuildAttachments(Vector<RenderPassAttachmentLoader>& attachmentTextureList);
        void                            BuildFrameBuffer(RenderPassLoader& renderPassLoader);

        void                            BeginRenderPass(VkCommandBuffer& commandBuffer, uint mipLevel = 0);
        void                            NextSubpass(VkCommandBuffer& commandBuffer);
        void                            BindViewPort(VkCommandBuffer& commandBuffer, uint drawMipLevel = 0);
        void                            BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet);
        void                            DrawMesh(VkCommandBuffer cmd, MeshDrawMessage& mesh);
        void                            EndRenderPass(VkCommandBuffer& commandBuffer);
        void                            Destroy();
};