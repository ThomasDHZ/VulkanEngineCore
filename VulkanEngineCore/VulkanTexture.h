#pragma once
#include "Platform.h"
#include "BufferSystem.h"
#include "VkGuid.h"

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
    kTextureType_CubeMap
};

struct VulkanTextureLoader
{
    Vector<byte>          TextureData;
    uint32                MipMapCount = UINT32_MAX;
    ivec3                 TextureSize = ivec3(UINT32_MAX);
    VkFormat              TextureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags    ImageType = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    ColorChannelEnum      ColorChannels = ColorChannelEnum::ChannelRGBA;
    VkImageLayout         TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkSamplerCreateInfo   SamplerCreateInfo;
    TextureTypeEnum       TextureType;
};

class VulkanTexture
{
private:
    ivec3                 m_textureSize = ivec3(UINT32_MAX);
    uint32                m_mipMapLevels = UINT32_MAX;

    VkImage               m_textureImage = VK_NULL_HANDLE;
    Vector<VkImageView>   m_textureViewList;
    VkSampler             m_textureSampler = VK_NULL_HANDLE;
    VmaAllocation         m_vmaTextureAllocation = VK_NULL_HANDLE;

    VkFormat              m_textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout         m_textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits m_sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelEnum      m_colorChannels = ColorChannelEnum::ChannelRGBA;

    void CreateTextureImage(VulkanTextureLoader& textureLoader);
    void CreateTextureView(VulkanTextureLoader& textureLoader);
    void GenerateMipmaps();

    bool IsDepthFormat(VkFormat format);
    bool HasStencilComponent(VkFormat format);
    void TransitionImageLayout(VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    void TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout newLayout, uint32 baseMip = 0, uint32 mipCount = VK_REMAINING_MIP_LEVELS, uint32 baseLayer = 0, uint32 layerCount = VK_REMAINING_ARRAY_LAYERS);

public:
    VulkanTexture();
    VulkanTexture(VulkanTextureLoader& textureLoader);
    ~VulkanTexture();
};