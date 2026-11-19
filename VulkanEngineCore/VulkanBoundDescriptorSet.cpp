#include "VulkanBoundDescriptorSet.h"
#include "VulkanSystem.h"

VulkanBoundDescriptorSet::VulkanBoundDescriptorSet()
{
}

VulkanBoundDescriptorSet::VulkanBoundDescriptorSet(uint32 inputAttachmentCount)
{
    m_inputAttachmentCount = inputAttachmentCount;
}

VulkanBoundDescriptorSet::~VulkanBoundDescriptorSet()
{
}

void VulkanBoundDescriptorSet::AddDescriptorSetBinding(const VulkanDescriptorSet& descriptorSetBinding)
{
    m_vulkanDescriptorSetList.emplace_back(descriptorSetBinding);
}

void VulkanBoundDescriptorSet::AddDescriptorSetBindings(const Vector<VulkanDescriptorSet>& descriptorSetBindingList)
{
    for (const auto descriptorSetBinding : descriptorSetBindingList)
    {
        m_vulkanDescriptorSetList.emplace_back(descriptorSetBinding);
    }
}

Vector<VkDescriptorPoolSize> VulkanBoundDescriptorSet::GetDescriptorPoolCounts()
{
    Vector<VkDescriptorPoolSize> descriptorPoolSizeList;
    UnorderedMap<VkDescriptorType, uint32> descriptorPoolCounts;
    if (m_inputAttachmentCount > 0) descriptorPoolCounts[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] = m_inputAttachmentCount;
    for (const auto& descriptorSet : m_vulkanDescriptorSetList)
    {
        descriptorPoolCounts[descriptorSet.DescriptorType] += descriptorSet.DescriptorPoolCount;
    }

    descriptorPoolSizeList.reserve(descriptorPoolCounts.size());
    for (const auto& [type, count] : descriptorPoolCounts)
    {
        if (count == 0) continue;
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
            {
                .type = type,
                .descriptorCount = count
            });
    }
    return descriptorPoolSizeList;
}

Vector<VkDescriptorBindingFlags> VulkanBoundDescriptorSet::GetDescriptorPoolFlags()
{
    Vector<VkDescriptorBindingFlags> flags;
    for (const auto& flag : m_vulkanDescriptorSetList)
    {
        flags.emplace_back(flag.DescriptorBindingFlags);
    }
    return flags;
}

Vector<VkDescriptorSetLayoutBinding> VulkanBoundDescriptorSet::GetDescriptorSetLayoutCreateInfo()
{
    Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList;
    for (const auto& descriptorSetLayoutInfo : m_vulkanDescriptorSetList)
    {
        descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = descriptorSetLayoutInfo.Binding,
                .descriptorType = descriptorSetLayoutInfo.DescriptorType,
                .descriptorCount = descriptorSetLayoutInfo.DescriptorCount,
                .stageFlags = descriptorSetLayoutInfo.StageFlags,
                .pImmutableSamplers = descriptorSetLayoutInfo.pImmutableSamplers
            });
    }
    return descriptorSetLayoutBindingList;
}

void VulkanBoundDescriptorSet::BuildDescriptorSets()
{ 
    Vector<VkDescriptorBindingFlags> flagList = GetDescriptorPoolFlags();
    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = static_cast<uint32>(flagList.size()),
        .pBindingFlags = flagList.data()
    };

    Vector<VkDescriptorPoolSize> descriptorPoolSizeList = GetDescriptorPoolCounts();
    VkDescriptorPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 64,
        .poolSizeCount = static_cast<uint32>(descriptorPoolSizeList.size()),
        .pPoolSizes = descriptorPoolSizeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkan.LogicalDevice(), &poolInfo, nullptr, &m_descriptorPool));

    Vector<VkDescriptorSetLayoutBinding> descriptorSetBindingList = GetDescriptorSetLayoutCreateInfo();
    VkDescriptorSetLayoutCreateInfo layoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32>(descriptorSetBindingList.size()),
        .pBindings = descriptorSetBindingList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkan.LogicalDevice(), &layoutInfo, nullptr, &m_descriptorSetLayoutList));

    VkDescriptorSetAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptorSetLayoutList
    };
    VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkan.LogicalDevice(), &allocInfo, &m_descriptorSetList));
}

void VulkanBoundDescriptorSet::UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding)
{
    VkDescriptorBufferInfo bufferUpdate = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(vulkanGpuBufferIndex).Buffer(),
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descriptorSetList,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferUpdate,
    };
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
}

void VulkanBoundDescriptorSet::UpdateTextureDescriptorSet(uint32 textureGpuBufferIndex, VulkanTexture& texture, uint binding)
{
    VkDescriptorImageInfo textureUpdate = {
      .sampler = texture.TextureSampler(),
      .imageView = texture.TextureViews().front(),
      .imageLayout = texture.m_colorChannels == ColorChannelEnum::ChannelR ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descriptorSetList,
        .dstBinding = binding,
        .dstArrayElement = textureGpuBufferIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &textureUpdate,
    };
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
}

VkDescriptorSet VulkanBoundDescriptorSet::DescriptorSet() const noexcept { return m_descriptorSetList; }
VkDescriptorSetLayout VulkanBoundDescriptorSet::DescriptorSetLayout() const noexcept { return m_descriptorSetLayoutList; }
VkDescriptorPool VulkanBoundDescriptorSet::DescriptorPool() const noexcept { return m_descriptorPool; }
