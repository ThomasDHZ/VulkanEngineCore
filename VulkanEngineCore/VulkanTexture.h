#pragma once
#include "Platform.h"
#include "BufferSystem.h"

enum TextureUsageTypeEnum : uint32
{
    kUsageType_Undefined,
    kUsageType_SwapChainTexture,
    kUsageType_OffscreenColorTexture,
    kUsageType_DepthBufferTexture,
    kUsageType_GBufferTexture,
    kUsageType_IrradianceTexture,
    kUsageType_PrefilterTexture,
    kUsageType_CubeMap,
    kUsageType_BRDFTexture,
    kUsageType_Texture
};

enum RenderAttachmentTypeEnum
{
    ColorRenderedTexture,
    InputAttachmentTexture,
    ResolveAttachmentTexture,
    DepthRenderedTexture,
    SkipSubPass
};

enum ColorChannelEnum
{
    ChannelR = 1,
    ChannelRG,
    ChannelRGB,
    ChannelRGBA
};

enum class TextureTypeEnum : uint32
{
    kTextureType_Undefined,
    kTextureType_ColorTexture,
    kTextureType_DepthTexture,
    kTextureType_StencilTexture,
    kTextureType_DataTexture,
    kTextureType_CubeMap,
    kTextureType_StorageTexture
};

struct RenderPassAttachmentLoader
{
    VkGuid                               RenderedTextureId = VkGuid();
    uint32                               MipMapCount = UINT32_MAX;
    TextureTypeEnum                      TextureType = TextureTypeEnum::kTextureType_Undefined;
    TextureUsageTypeEnum                 TextureUsageType = kUsageType_Undefined;
    Vector<RenderAttachmentTypeEnum>     RenderAttachmentTypes = Vector<RenderAttachmentTypeEnum>();
    VkFormat                             TextureByteFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkAttachmentLoadOp                   LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp                  StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkSamplerCreateInfo                  SamplerCreateInfo = VkSamplerCreateInfo();
    VkImageLayout                        FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSampleCountFlagBits                SampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool                                 UseMipMaps = false;
    bool                                 IsSkyBox = false;
};

struct VulkanTextureLoader
{
    Vector<byte>          TextureData;
    ivec3                 TextureDimensions = ivec3(UINT32_MAX);
    VkSamplerCreateInfo   SamplerCreateInfo;
    uint32                MipMapCount = UINT32_MAX;
    ColorChannelEnum      ColorChannels = ColorChannelEnum::ChannelRGBA;
    VkImageLayout         TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkFormat              TextureByteFormat = VK_FORMAT_UNDEFINED;
    TextureTypeEnum       TextureType;
    bool                  IsRenderPassAttachment;
    bool                  IsCubeMap;
    bool                  UseMipMaps;
};

class DLL_EXPORT VulkanTexture
{
private:

    void CreateTextureImage(const Vector<byte>& textureData);
    void CreateTextureView();
    void CreateTextureSampler(VkSamplerCreateInfo& samplerCreateInfo);
    void UploadTextureDataAndTransition(const Vector<byte>& textureData);
    void GenerateMipmaps(VkCommandBuffer& cmd);

    uint32 MaxMipLevels(uint32 mipMapCount, bool usingMips);
    size_t GetMipOffset(uint32_t mipLevel) const;
    uint32 GetBlockSizeInBytes(VkFormat format);

public:
    ivec3                 m_textureSize = ivec3(UINT32_MAX, UINT32_MAX, 1);
    uint32                m_mipMapLevels = UINT32_MAX;
    uint32                m_bytesPerChannel = UINT32_MAX;

    VkImage               m_textureImage = VK_NULL_HANDLE;
    Vector<VkImageView>   m_textureViewList;
    VkSampler             m_textureSampler = VK_NULL_HANDLE;
    VmaAllocation         m_vmaTextureAllocation = VK_NULL_HANDLE;

    VkFormat              m_textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout         m_textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelEnum      m_colorChannels = ColorChannelEnum::ChannelRGBA;
    TextureTypeEnum       m_textureType = TextureTypeEnum::kTextureType_Undefined;
    bool                  m_isDepthTexture = false;
    bool                  m_isStencil = false;
    bool                  m_isRenderPassAttachment = false;
    bool                  m_isCubeMap = false;
    bool                  m_isCompressedFormat = false;

    VulkanTexture();
    VulkanTexture(VulkanTextureLoader& textureLoader);
    VulkanTexture(ivec2& attachmentSize, RenderPassAttachmentLoader& attachment);
    ~VulkanTexture();

    void TransitionImageLayout(VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    void TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout newLayout, uint32 baseMip = 0, uint32 mipCount = VK_REMAINING_MIP_LEVELS, uint32 baseLayer = 0, uint32 layerCount = VK_REMAINING_ARRAY_LAYERS);
    void DestroyTexture();

    static bool         IsDepthFormat(VkFormat format);
    static bool         IsStencilFormat(VkFormat format);
    ivec3               TextureSize();

    [[nodiscard]] VkImage             TextureImage()                        const noexcept;
    [[nodiscard]] Vector<VkImageView> TextureViews()                        const noexcept;
    [[nodiscard]] VkSampler           TextureSampler()                      const noexcept;
    [[nodiscard]] VkImageLayout       TextureImageLayout()                  const noexcept;
    [[nodiscard]] uint32              MipMapLevels()                        const noexcept;
    [[nodiscard]] bool                IsDepthTexture()                      const noexcept;
    [[nodiscard]] bool                IsStencil()                           const noexcept;
    [[nodiscard]] bool                IsRenderPassAttachment()              const noexcept;
    [[nodiscard]] bool                IsCubeMap()                           const noexcept;
    [[nodiscard]] uint32              TextureArrayLayers()                  const noexcept;
};