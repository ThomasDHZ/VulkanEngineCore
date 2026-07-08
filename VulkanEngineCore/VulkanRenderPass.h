#pragma once
#include "Platform.h"

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

struct VulkanRenderPass
{
    VkGuid                               RenderPassId = VkGuid();
    ivec2                                RenderPassResolution = ivec2(INT32_MAX, INT32_MAX);
    VkRenderPass                         RenderPass = VK_NULL_HANDLE;
    Vector<VkFramebuffer>                FrameBufferList;
    Vector<Vector<VulkanSubPass>>        VulkanSubPassList;
    Vector<VkClearValue>                 ClearValueList;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseCubeMapMultiView = false;
    bool                                 IsCubeMapRenderPass = false;
};

