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

class Texturea
{
    TextureGuid           m_textureGuid = TextureGuid();
    size_t                m_textureId = SIZE_MAX;
    int                   m_width = 1;
    int                   m_height = 1;
    int                   m_depth = 1;
    uint32                mipMapLevels = 0;

    VkImage               textureImage = VK_NULL_HANDLE;
    Vector<VkImageView>   textureViewList = Vector<VkImageView>(1, VK_NULL_HANDLE);
    VkSampler             textureSampler = VK_NULL_HANDLE;
    VkDescriptorSet       ImGuiDescriptorSet = VK_NULL_HANDLE;
    VmaAllocation         TextureAllocation = VK_NULL_HANDLE;

    TextureTypeEnum       textureType = TextureTypeEnum::kTextureType_Undefined;
    VkFormat              textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout         textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelEnum      colorChannels = ColorChannelEnum::ChannelRGBA;
};