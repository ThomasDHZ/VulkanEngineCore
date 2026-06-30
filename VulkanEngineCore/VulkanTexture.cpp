#include "VulkanTexture.h"

VulkanTexture::VulkanTexture()
{
}

VulkanTexture::VulkanTexture(VulkanTextureLoader& texture)
{
	m_textureSize = texture.TextureSize;
	m_mipMapLevels = texture.MipMapCount == UINT32_MAX ? static_cast<uint32>(std::floor(std::log2(std::max(m_textureSize.x, m_textureSize.y)))) + 1 : 1;
	m_textureImage = VK_NULL_HANDLE;
	m_textureViewList;
	m_textureSampler = VK_NULL_HANDLE;
	m_vmaTextureAllocation = VK_NULL_HANDLE;
	m_textureByteFormat = texture.TextureByteFormat;
	m_textureImageLayout = texture.TextureImageLayout;
	m_sampleCount = texture.SampleCount;
	m_colorChannels = texture.ColorChannels;

	CreateTextureImage(texture);
	CreateTextureView(texture);
	VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkan.LogicalDevice(), &texture.SamplerCreateInfo, nullptr, &m_textureSampler));
}

VulkanTexture::~VulkanTexture()
{
}

void VulkanTexture::CreateTextureImage(VulkanTextureLoader& texture)
{
	VkImageCreateInfo imageCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = texture.TextureType == TextureTypeEnum::kTextureType_CubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageViewCreateFlags>(0),
		.imageType = VK_IMAGE_TYPE_2D,
		.format = texture.TextureByteFormat,
		.extent =
		{
			.width =  static_cast<uint32>(m_textureSize.x),
			.height = static_cast<uint32>(m_textureSize.y),
			.depth =  static_cast<uint32>(m_textureSize.z),
		},
		.mipLevels = m_mipMapLevels,
		.arrayLayers = texture.TextureType == TextureTypeEnum::kTextureType_CubeMap ? static_cast<uint>(6) : static_cast<uint>(1),
		.samples = m_sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = m_textureImageLayout
	};
	if (m_mipMapLevels > 1) imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	if (imageCreateInfo.extent.width * imageCreateInfo.extent.height > 1024 * 1024)
	{
		allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}

	VULKAN_THROW_IF_FAIL(vmaCreateImage(bufferSystem.VmaAllocatorHandle(), &imageCreateInfo, &allocInfo, &m_textureImage, &m_vmaTextureAllocation, nullptr));
	if (texture.TextureData.empty())
	{
		m_textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		return;
	}

	VkBufferCreateInfo stagingCI
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = texture.TextureData.size(),
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};

	VmaAllocationCreateInfo stagingAllocCI
	{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingAlloc = VK_NULL_HANDLE;
	VmaAllocationInfo allocInfoOut{};
	VULKAN_THROW_IF_FAIL(vmaCreateBuffer(bufferSystem.VmaAllocatorHandle(), &stagingCI, &stagingAllocCI, &stagingBuffer, &stagingAlloc, &allocInfoOut));
	memcpy(allocInfoOut.pMappedData, texture.TextureData.data(), texture.TextureData.size());
	VULKAN_THROW_IF_FAIL(vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), stagingAlloc, 0, texture.TextureData.size()));

	VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();
	TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_REMAINING_MIP_LEVELS, 0, imageCreateInfo.arrayLayers);

	Vector<VkBufferImageCopy> copyRegions;
	copyRegions.reserve(imageCreateInfo.arrayLayers);
	VkDeviceSize layerSize = texture.TextureData.size() / imageCreateInfo.arrayLayers;
	for (uint32_t layer = 0; layer < imageCreateInfo.arrayLayers; ++layer)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = layer * layerSize;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = layer,
			.layerCount = 1
		};
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent =
		{
			static_cast<uint32>(texture.TextureSize.x),
			static_cast<uint32>(texture.TextureSize.y),
			static_cast<uint32>(texture.TextureSize.z)
		};
		copyRegions.push_back(region);
	}
	vkCmdCopyBufferToImage(cmd, stagingBuffer, m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

	if (m_mipMapLevels > 1) GenerateMipmaps();
	else TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_REMAINING_MIP_LEVELS, 0, imageCreateInfo.arrayLayers);

	vulkan.CommandBuffer().EndSingleUseCommand(cmd);
	vmaDestroyBuffer(bufferSystem.VmaAllocatorHandle(), stagingBuffer, stagingAlloc);
}

void VulkanTexture::CreateTextureView(VulkanTextureLoader& texture)
{
	VkImageAspectFlags aspect = texture.ImageType;
	if (aspect == 0)
	{
		std::cout << "CreateTextureView: imageAspectFlags not set ? using auto-detect." << std::endl;

		bool isDepthFormat = (m_textureByteFormat >= VK_FORMAT_D16_UNORM &&
							  m_textureByteFormat <= VK_FORMAT_D32_SFLOAT_S8_UINT) ||
							 (m_textureByteFormat == VK_FORMAT_X8_D24_UNORM_PACK32);

		if (isDepthFormat)
		{
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (m_textureByteFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
				m_textureByteFormat == VK_FORMAT_D24_UNORM_S8_UINT)
			{
				aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	for (uint32 mip = 0; mip < m_mipMapLevels; ++mip)
	{
		VkImageView imageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_textureImage;

		if (texture.TextureType == TextureTypeEnum::kTextureType_CubeMap)
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}

		viewInfo.format = m_textureByteFormat;
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = mip;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = texture.TextureType == TextureTypeEnum::kTextureType_CubeMap ? 6u : 1u;

		VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkan.LogicalDevice(), &viewInfo, nullptr, &imageView));
		m_textureViewList.emplace_back(imageView);
	}
}

void VulkanTexture::GenerateMipmaps()
{
	if (m_mipMapLevels <= 1)
	{
		return;
	}

	VkFormatProperties props{};
	vkGetPhysicalDeviceFormatProperties(vulkan.PhysicalDevice(), m_textureByteFormat, &props);
	bool supportsLinear = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
	if (!supportsLinear)
	{
		std::cout << "[WARNING] Format " << m_textureByteFormat << " does not support linear filtering → mipmaps will use nearest or be incorrect\n";
	}

	VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();

	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	if (IsDepthFormat(m_textureByteFormat))
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (HasStencilComponent(m_textureByteFormat))
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

	int32 currentWidth = m_textureSize.x;
	int32 currentHeight = m_textureSize.y;
	int32 currentDepth = m_textureSize.z;
	for (uint32_t dstMip = 1; dstMip < m_mipMapLevels; ++dstMip)
	{
		uint32_t srcMip = dstMip - 1;

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
	vulkan.CommandBuffer().EndSingleUseCommand(cmd);
}

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

bool VulkanTexture::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D16_UNORM_S8_UINT ||
		   format == VK_FORMAT_D24_UNORM_S8_UINT ||
		   format == VK_FORMAT_D32_SFLOAT_S8_UINT;
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
		if (HasStencilComponent(m_textureByteFormat))
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
