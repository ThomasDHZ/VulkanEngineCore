#pragma once
#include "Platform.h"
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
    bool                                 UseGlobalBindlessSet = false;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;
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

class DLL_EXPORT VulkanRenderPass
{
private:
    Vector<VkAttachmentDescription> BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass);
    Vector<VulkanTexture>           BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass);
    void                            BuildFrameBuffer(VulkanRenderPass& vulkanRenderPass);

    void                            BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, ivec2 renderPassResolution, uint mipLevel = 0);
    void                            BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel = 0);
    void                            BindViewPort(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel = 0);
    void                            BindViewPort(VkCommandBuffer& commandBuffer, ivec2 renderPassResolution, uint mipLevel = 0);
    void                            EndRenderPass(VkCommandBuffer& commandBuffer);

public:

    VkGuid                               RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(INT32_MAX, INT32_MAX);
    VkRenderPass                         RenderPass = VK_NULL_HANDLE;
    Vector<VkFramebuffer>                FrameBufferList;
    Vector<Vector<VulkanSubPass>>        VulkanSubPassList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;

    VulkanRenderPass();
    ~VulkanRenderPass();

    void BuildRenderPass(const RenderPassLoader& renderPassJsonLoader);
    void Draw(VkCommandBuffer& commandBuffer);
};

