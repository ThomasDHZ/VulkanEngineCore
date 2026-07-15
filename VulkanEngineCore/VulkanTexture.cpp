#include "VulkanTexture.h"
#include "BufferSystem.h"

VulkanTexture::VulkanTexture()
{
}

VulkanTexture::VulkanTexture(VulkanTextureLoader& textureLoader)
{
	m_textureSize = textureLoader.TextureDimensions;
	m_mipMapLevels = MaxMipLevels(textureLoader.MipMapCount, textureLoader.UseMipMaps);
	m_textureByteFormat = textureLoader.TextureByteFormat;
	m_textureImageLayout = textureLoader.TextureImageLayout;
	m_sampleCount = textureLoader.SampleCount;
	m_colorChannels = textureLoader.ColorChannels;
	m_textureType = textureLoader.TextureType;
	m_isCubeMap = textureLoader.IsCubeMap;
	m_isDepthTexture = IsDepthFormat(textureLoader.TextureByteFormat);
	m_isStencil = IsStencilFormat(textureLoader.TextureByteFormat);
	m_isRenderPassAttachment = false;

	CreateTextureImage(textureLoader.TextureData);
	CreateTextureView();
	CreateTextureSampler(textureLoader.SamplerCreateInfo);
}

VulkanTexture::VulkanTexture(ivec2& attachmentSize, RenderPassAttachmentLoader& attachment)
{
	m_textureSize = ivec3(attachmentSize.x, attachmentSize.y, 1);
	m_mipMapLevels = MaxMipLevels(attachment.MipMapCount, attachment.UseMipMaps);
	m_textureByteFormat = attachment.TextureByteFormat;
	m_textureImageLayout = attachment.FinalLayout;
	m_sampleCount = attachment.SampleCount;
	m_textureType = attachment.TextureType;
	m_isCubeMap = attachment.TextureType == TextureTypeEnum::kTextureType_CubeMap ? true : false;
	m_isDepthTexture = IsDepthFormat(attachment.TextureByteFormat);
	m_isStencil = IsStencilFormat(attachment.TextureByteFormat);
	m_isRenderPassAttachment = true;

	switch (attachment.TextureUsageType)
	{
		case kUsageType_DepthBufferTexture:     m_textureImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
		case kUsageType_GBufferTexture:         m_textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_IrradianceTexture:      m_textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_PrefilterTexture:       m_textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_OffscreenColorTexture:  m_textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_SwapChainTexture:       m_textureImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  break;
		case kUsageType_CubeMap:				m_textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
		case kUsageType_BRDFTexture:			m_textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
	}

	Vector<byte> dummyTextureData;
	CreateTextureImage(dummyTextureData);
	CreateTextureView();
	CreateTextureSampler(attachment.SamplerCreateInfo);
}

VulkanTexture::~VulkanTexture()
{
}

void VulkanTexture::CreateTextureImage(const Vector<byte>& textureData)
{
	VkImageCreateInfo imageInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = m_isCubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = m_textureByteFormat,
		.extent =
		{
			.width = static_cast<uint32>(m_textureSize.x),
			.height = static_cast<uint32>(m_textureSize.y),
			.depth = static_cast<uint32>(m_textureSize.z),
		},
		.mipLevels = m_mipMapLevels,
		.arrayLayers = m_isCubeMap ? 6u : 1u,
		.samples = m_sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE, 
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	if (m_isCubeMap) imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	if (m_isRenderPassAttachment) imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	if (m_isRenderPassAttachment) imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (m_mipMapLevels > 1 || m_isRenderPassAttachment) imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (IsDepthFormat(m_textureByteFormat)) imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	VmaAllocationCreateInfo vmaAllocationCreateInfo =
	{
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};
	if (m_textureSize.x * m_textureSize.y * m_textureSize.z > 1024 * 1024) vmaAllocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VULKAN_THROW_IF_FAIL(vmaCreateImage(bufferSystem.VmaAllocatorHandle(), &imageInfo, &vmaAllocationCreateInfo, &m_textureImage, &m_vmaTextureAllocation, nullptr));
	if (!m_isRenderPassAttachment) UploadTextureDataAndTransition(textureData);
}

void VulkanTexture::CreateTextureView()
{
	VkImageAspectFlags aspectMask = IsDepthFormat(m_textureByteFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (IsStencilFormat(m_textureByteFormat)) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;


	for (uint32 mip = 0; mip < m_mipMapLevels; mip++)
	{
		VkImageView imageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo viewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_textureImage,
			.viewType = m_isCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
			.format = m_textureByteFormat,
			.subresourceRange
			{
				.aspectMask = aspectMask,
				.baseMipLevel = mip,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = m_isCubeMap ? 6u : 1u
			}
		};
		if (vkCreateImageView(vulkan.LogicalDevice(), &viewInfo, nullptr, &imageView))
		{
			for (auto textureView : m_textureViewList)
			{
				vkDestroyImageView(vulkan.LogicalDevice(), textureView, nullptr);
			}
			vmaDestroyImage(bufferSystem.VmaAllocatorHandle(), m_textureImage, m_vmaTextureAllocation);
			return;
		}
		m_textureViewList.emplace_back(imageView);
	}
}

void VulkanTexture::CreateTextureSampler(VkSamplerCreateInfo& samplerCreateInfo)
{
	if (vkCreateSampler(vulkan.LogicalDevice(), &samplerCreateInfo, nullptr, &m_textureSampler) != VK_SUCCESS)
	{
		vkDestroySampler(vulkan.LogicalDevice(), m_textureSampler, nullptr);
		for (auto textureView : m_textureViewList)
		{
			vkDestroyImageView(vulkan.LogicalDevice(), textureView, nullptr);
		}
		vmaDestroyImage(bufferSystem.VmaAllocatorHandle(), m_textureImage, m_vmaTextureAllocation);
		return;
	}
}

void VulkanTexture::UploadTextureDataAndTransition(const Vector<byte>& textureData)
{
	if (textureData.empty() || m_isRenderPassAttachment)
		return;


	VkBufferCreateInfo stagingInfo = VkBufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = textureData.size(),
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	};

	VmaAllocationCreateInfo stagingAllocInfo = VmaAllocationCreateInfo
	{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};

	VmaAllocationInfo allocInfo{};
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingAllocation = VK_NULL_HANDLE;
	VULKAN_THROW_IF_FAIL(vmaCreateBuffer(bufferSystem.VmaAllocatorHandle(), &stagingInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &allocInfo));

	void* mapped = allocInfo.pMappedData;
	if (!mapped) VULKAN_THROW_IF_FAIL(vmaMapMemory(bufferSystem.VmaAllocatorHandle(), stagingAllocation, &mapped));

	memcpy(mapped, textureData.data(), textureData.size());
	vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), stagingAllocation, 0, VK_WHOLE_SIZE);

	if (!allocInfo.pMappedData) vmaUnmapMemory(bufferSystem.VmaAllocatorHandle(), stagingAllocation);

	uint32 arrayLayers = m_isCubeMap ? 6u : 1u;
	VkDeviceSize layerSize = textureData.size() / arrayLayers;
	VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();
	TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_REMAINING_MIP_LEVELS, 0, arrayLayers);
	std::vector<VkBufferImageCopy> copyRegions;
	copyRegions.reserve(arrayLayers);
	for (uint32 layer = 0; layer < arrayLayers; ++layer)
	{
		copyRegions.push_back(VkBufferImageCopy
			{
				.bufferOffset = layer * layerSize,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = 
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = layer,
					.layerCount = 1
				},
				.imageOffset = {0, 0, 0},
				.imageExtent = 
				{
					static_cast<uint32_t>(m_textureSize.x),
					static_cast<uint32_t>(m_textureSize.y),
					static_cast<uint32_t>(m_textureSize.z)
				}
			});
	}
	vkCmdCopyBufferToImage(cmd, stagingBuffer, m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

	if (m_mipMapLevels > 1) GenerateMipmaps(cmd);
	else TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_REMAINING_MIP_LEVELS, 0, arrayLayers);

	vulkan.CommandBuffer().EndSingleUseCommand(cmd);
	vmaDestroyBuffer(bufferSystem.VmaAllocatorHandle(), stagingBuffer, stagingAllocation);
}

void VulkanTexture::GenerateMipmaps(VkCommandBuffer& cmd)
{
	if (m_mipMapLevels <= 1) return;

	VkFormatProperties props{};
	vkGetPhysicalDeviceFormatProperties(vulkan.PhysicalDevice(), m_textureByteFormat, &props);
	bool supportsLinear = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
	if (!supportsLinear)
	{
		std::cout << "[WARNING] Format " << m_textureByteFormat << " does not support linear filtering → mipmaps will use nearest or be incorrect\n";
	}

	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (IsDepthFormat(m_textureByteFormat))
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (IsStencilFormat(m_textureByteFormat))
		{
			aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_textureImage;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	int currentWidth = m_textureSize.x;
	int currentHeight = m_textureSize.y;
	int currentDepth = m_textureSize.z;
	for (uint32 dstMip = 1; dstMip < m_mipMapLevels; ++dstMip)
	{
		uint32 srcMip = dstMip - 1;

		barrier.subresourceRange.baseMipLevel = srcMip;
		barrier.subresourceRange.levelCount = 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit
		{
			.srcSubresource =
			{
				.aspectMask = aspect,
				.mipLevel = srcMip,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
			.srcOffsets =
			{
				{
					.x = 0,
					.y = 0,
					.z = 0
				},
				{ 
					.x = currentWidth, 
					.y = currentHeight,
					.z = currentDepth }
			},
			.dstSubresource =
			{
				.aspectMask = aspect,
				.mipLevel = dstMip,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
			.dstOffsets
			{
				{ 
					.x = 0, 
					.y = 0, 
					.z = 0 
				},
				{
					.x = std::max(1, currentWidth / 2),
					.y = std::max(1, currentHeight / 2),
					.z = std::max(1, currentDepth / 2),
				}
			}
		};
		vkCmdBlitImage(cmd, m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, supportsLinear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		currentWidth = std::max(1, currentWidth / 2);
		currentHeight = std::max(1, currentHeight / 2);
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

uint32 VulkanTexture::MaxMipLevels(uint32 mipMapCount, bool usingMips)
{
	uint32 maxMipCount = static_cast<uint32>(std::floor(std::log2(std::max(m_textureSize.x, m_textureSize.y)))) + 1;
	if (usingMips) return mipMapCount > maxMipCount ? maxMipCount : mipMapCount;
	return 1;
}

void VulkanTexture::TransitionImageLayout(VkImageLayout newLayout, uint32 baseMipLevel, uint32 levelCount)
{
	VkCommandBuffer commandBuffer = vulkan.CommandBuffer().BeginSingleUseCommand();
	TransitionImageLayout(commandBuffer, newLayout);
	vulkan.CommandBuffer().EndSingleUseCommand(commandBuffer);
	m_textureImageLayout = newLayout;
}

void VulkanTexture::TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout newLayout, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
{
	if (m_textureImage == VK_NULL_HANDLE) return;

	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (IsDepthFormat(m_textureByteFormat))
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (IsStencilFormat(m_textureByteFormat))
		{
			aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = m_textureImageLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_textureImage;
	barrier.subresourceRange = { aspect, baseMip, mipCount, baseLayer, layerCount };

	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkAccessFlags srcAccess = 0;
	VkAccessFlags dstAccess = 0;

	if (m_textureImageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			srcAccess = 0;
			dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			srcAccess = 0;
			dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
	}
	else if (m_textureImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstAccess = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
	}
	else if (m_textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
	{
		srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	m_textureImageLayout = newLayout;
}

void VulkanTexture::DestroyTexture()
{
	if (m_textureSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(vulkan.LogicalDevice(), m_textureSampler, nullptr);
		m_textureSampler = VK_NULL_HANDLE;
	}

	for (VkImageView view : m_textureViewList)
	{
		if (view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(vulkan.LogicalDevice(), view, nullptr);
		}
	}
	m_textureViewList.clear();

	if (m_textureImage != VK_NULL_HANDLE)
	{
		if (m_vmaTextureAllocation != VK_NULL_HANDLE)
		{
			vmaDestroyImage(bufferSystem.VmaAllocatorHandle(), m_textureImage, m_vmaTextureAllocation);
			m_vmaTextureAllocation = VK_NULL_HANDLE;
		}
		else
		{
			vkDestroyImage(vulkan.LogicalDevice(), m_textureImage, nullptr);
		}
		m_textureImage = VK_NULL_HANDLE;
	}
}

VkImage				VulkanTexture::TextureImage()			const noexcept { return m_textureImage; }
Vector<VkImageView> VulkanTexture::TextureViews()			const noexcept { return m_textureViewList; }
VkSampler			VulkanTexture::TextureSampler()			const noexcept { return m_textureSampler; }
ivec3				VulkanTexture::TextureSize()			{ return m_textureSize; }
VkImageLayout       VulkanTexture::TextureImageLayout()		const noexcept { return m_textureImageLayout; }
uint32				VulkanTexture::MipMapLevels()			const noexcept { return m_mipMapLevels; }
bool				VulkanTexture::IsDepthTexture()			const noexcept { return m_isDepthTexture; }
bool				VulkanTexture::IsStencil()				const noexcept { return m_isStencil; }
bool				VulkanTexture::IsRenderPassAttachment() const noexcept { return m_isRenderPassAttachment; }
bool				VulkanTexture::IsCubeMap()				const noexcept { return m_isCubeMap; }
uint32				VulkanTexture::TextureArrayLayers()		const noexcept { return m_isCubeMap ? 6u : 1u; };

bool VulkanTexture::IsDepthFormat(VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return true;
		default:
			return false;
	}
}

bool VulkanTexture::IsStencilFormat(VkFormat format)
{
	return format == VK_FORMAT_D16_UNORM_S8_UINT ||
		   format == VK_FORMAT_D24_UNORM_S8_UINT ||
		   format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}
