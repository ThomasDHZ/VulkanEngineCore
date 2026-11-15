#include "MemoryPool.h"
#include <BufferSystem.h>
#include <MemorySystem.h>


MemoryPool::MemoryPool()
{
}

MemoryPool::MemoryPool(uint32 objectSize, uint32 initialCapacity, uint32 descriptorSetBinding)
{
    m_memoryPoolHeader = MemoryPoolHeader{
        .ActiveCount = 0,
        .Capacity = initialCapacity,
        .SizeOfObject = objectSize,
        .IsSlotActive = Vector<byte>(initialCapacity, 0x00),
        .FreeIndices = {},
        .IsDirty = true
    };
    CreateMemoryPool(nullptr, 0, descriptorSetBinding);
}

MemoryPool::MemoryPool(void* initialData, uint32 objectSize, uint32 objectCount, uint32 initialCapacity, uint32 descriptorSetBinding)
{
    m_memoryPoolHeader = MemoryPoolHeader{
    .ActiveCount = 0,
    .Capacity = initialCapacity,
    .SizeOfObject = objectSize,
    .IsSlotActive = Vector<byte>(initialCapacity, 0x00),
    .FreeIndices = {},
    .IsDirty = true
    };
    CreateMemoryPool(initialData, objectCount, descriptorSetBinding);
}

void MemoryPool::CreateMemoryPool(void* data, size_t count, uint32 binding)

{
    m_memoryPoolTotalSize = sizeof(GpuMemoryPoolHeader) + static_cast<size_t>(m_memoryPoolHeader.Capacity) * m_memoryPoolHeader.SizeOfObject;
    m_memoryPoolGpuHeader = GpuMemoryPoolHeader
    {
        .FirstObjectOffset = sizeof(GpuMemoryPoolHeader),
        .ObjectCount = 0,
        .ObjectSize = m_memoryPoolHeader.SizeOfObject
    };
    m_memoryPoolDescriptorSetBinding = binding;
    m_memoryPoolBufferIndex = bufferSystem.CreateDynamicBuffer(nullptr, m_memoryPoolTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex);
    m_mappedMemoryPoolBufferPtr = buffer.BufferMappedData();

    std::memset(m_mappedMemoryPoolBufferPtr, 0x00, m_memoryPoolTotalSize);
    memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));
    if (data && count > 0)
    {
        uint32 toCopy = static_cast<uint32>(std::min(count, static_cast<size_t>(m_memoryPoolHeader.Capacity)));
        size_t bytes = static_cast<size_t>(toCopy) * m_memoryPoolHeader.SizeOfObject;
        memcpy(static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(GpuMemoryPoolHeader), data, bytes);

        m_memoryPoolHeader.ActiveCount = toCopy;
        for (uint32 x = 0; x < toCopy; ++x) m_memoryPoolHeader.IsSlotActive[x] = 1;

        m_memoryPoolGpuHeader.ObjectCount = toCopy;
        memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));
    }
    vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), 0, m_memoryPoolTotalSize);
    CreateBindlessDescriptorSet();
}

void MemoryPool::CreateBindlessDescriptorSet()
{
    VkDescriptorBufferInfo  bindlessDataBuffer = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex).Buffer(),
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    Vector<VkDescriptorPoolSize> poolSizes = { VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512 } };
    Vector<VkDescriptorSetLayoutBinding> bindings = { VkDescriptorSetLayoutBinding{ m_memoryPoolDescriptorSetBinding , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  1, VK_SHADER_STAGE_ALL}, };
    Vector<VkDescriptorBindingFlags> flags = { VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT } };

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = static_cast<uint32>(flags.size()),
        .pBindingFlags = flags.data()
    };

    VkDescriptorPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 64,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkan.LogicalDevice(), &poolInfo, nullptr, &m_memoryPoolBindlessPool));

    VkDescriptorSetLayoutCreateInfo layoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkan.LogicalDevice(), &layoutInfo, nullptr, &m_memoryPoolBindlessDescriptorSetLayout));

    VkDescriptorSetAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_memoryPoolBindlessPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_memoryPoolBindlessDescriptorSetLayout
    };
    VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkan.LogicalDevice(), &allocInfo, &m_memoryPoolBindlessDescriptorSet));

    Vector<VkWriteDescriptorSet> writeDescriptorSetList = Vector<VkWriteDescriptorSet>
    {
        VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_memoryPoolBindlessDescriptorSet,
                .dstBinding = m_memoryPoolDescriptorSetBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &bindlessDataBuffer,
            }
    };
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), static_cast<uint32>(writeDescriptorSetList.size()), writeDescriptorSetList.data(), 0, nullptr);
}

void MemoryPool::UpdateTextureDescriptorSet(uint32 textureGpuBufferIndex, VulkanTexture& texture)
{
    VkDescriptorImageInfo textureUpdate = {
      .sampler = texture.TextureSampler(),
      .imageView = texture.TextureViews().front(),
      .imageLayout = texture.m_colorChannels == ColorChannelEnum::ChannelR ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_memoryPoolBindlessDescriptorSet,
        .dstBinding = m_memoryPoolDescriptorSetBinding,
        .dstArrayElement = textureGpuBufferIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &textureUpdate,
    };
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
}

void MemoryPool::ResizeMemoryPool(uint32 newCapacity)
{
    void* oldMapped = m_mappedMemoryPoolBufferPtr;
    uint32 oldBufferId = m_memoryPoolBufferIndex;
    UpdateMemoryPoolHeader(newCapacity);

    uint32 newBufferId = bufferSystem.CreateDynamicBuffer(nullptr, m_memoryPoolTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
    m_mappedMemoryPoolBufferPtr = newBuf.BufferMappedData();

    std::memset(m_mappedMemoryPoolBufferPtr, 0x00, m_memoryPoolTotalSize);
    memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));

    size_t bytesToCopy = static_cast<size_t>(m_memoryPoolHeader.ActiveCount) * m_memoryPoolHeader.SizeOfObject;
    if (bytesToCopy > 0 && oldMapped)
    {
        byte* dst = static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(GpuMemoryPoolHeader);
        byte* src = static_cast<byte*>(oldMapped) + sizeof(GpuMemoryPoolHeader);
        memcpy(dst, src, bytesToCopy);
    }

    vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), newBuf.BufferAllocation(), 0, m_memoryPoolTotalSize);
    if (oldBufferId != UINT32_MAX) bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(oldBufferId));

    m_memoryPoolBufferIndex = newBufferId;
    m_isDescriptorSetDirty = true;
    m_isHeaderDirty = true;
}

void MemoryPool::RefreshGpuHeaderCounts()
{
    m_memoryPoolGpuHeader.ObjectCount = m_memoryPoolHeader.ActiveCount;
    m_isHeaderDirty = true;
}

void MemoryPool::RebuildFreeIndices()
{
    m_memoryPoolHeader.FreeIndices.clear();
    for (uint32 x = 0; x < m_memoryPoolHeader.Capacity; ++x)
    {
        if (m_memoryPoolHeader.IsSlotActive[x] == 0) m_memoryPoolHeader.FreeIndices.push_back(x);
    }
}

uint32 MemoryPool::AllocateObject()
{
    if (!m_memoryPoolHeader.FreeIndices.empty())
    {
        uint32 index = m_memoryPoolHeader.FreeIndices.back();
        m_memoryPoolHeader.FreeIndices.pop_back();
        m_memoryPoolHeader.IsSlotActive[index] = 1;

        if (index + 1 > m_memoryPoolHeader.ActiveCount) m_memoryPoolHeader.ActiveCount = index + 1;
        m_memoryPoolHeader.IsDirty = true;
        RefreshGpuHeaderCounts();
        return index;
    }
    if (m_memoryPoolHeader.ActiveCount == m_memoryPoolHeader.Capacity) ResizeMemoryPool(std::max(m_memoryPoolHeader.Capacity * 2, 16u));

    uint32 index = m_memoryPoolHeader.ActiveCount++;
    m_memoryPoolHeader.IsSlotActive[index] = 0x01;
    m_memoryPoolHeader.IsDirty = true;
    RefreshGpuHeaderCounts();
    return index;
}

uint32 MemoryPool::AddTextureToMemoryPool(VulkanTexture& texture)
{
    TextureMetadataHeader textureMetaDataHeader = TextureMetadataHeader
    {
        .Width = static_cast<uint32>(texture.TextureSize().x),
        .Height = static_cast<uint32>(texture.TextureSize().y),
        .Depth = static_cast<uint32>(texture.TextureSize().z),
        .MipLevels = texture.MipMapLevels(),
        .LayerCount = texture.TextureArrayLayers(),
        .Format = (uint32)texture.TextureImageLayout(),
        .Type = static_cast<uint32>(texture.IsCubeMap() ? 1 : 0)
    };

    uint32 gpuTextureIndex = AllocateObject();
    if (gpuTextureIndex >= m_memoryPoolHeader.Capacity || gpuTextureIndex >= m_memoryPoolHeader.IsSlotActive.size() || !m_memoryPoolHeader.IsSlotActive[gpuTextureIndex]) throw std::runtime_error("Invalid texture metadata index");

    uint32 offset = sizeof(GpuMemoryPoolHeader) + (gpuTextureIndex * sizeof(TextureMetadataHeader));
    memcpy(static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + offset, static_cast<void*>(&textureMetaDataHeader), sizeof(TextureMetadataHeader));
    UpdateTextureDescriptorSet(gpuTextureIndex, texture);
    return gpuTextureIndex;
}

void MemoryPool::UpdateMemoryPool()
{
    if (!m_mappedMemoryPoolBufferPtr) return;

    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex);
    const VkDeviceSize atom = vulkan.Device().GetPhysicalDeviceProperties(vulkan.PhysicalDevice()).limits.nonCoherentAtomSize;
    const VkDeviceSize allocationSize = m_memoryPoolTotalSize;

    auto flushRange = [&](VkDeviceSize start, VkDeviceSize len)
        {
            if (len == 0 || start >= allocationSize) return;
            VkDeviceSize end = std::min(start + len, allocationSize);
            VkDeviceSize alignedStart = start & ~(atom - 1);
            VkDeviceSize alignedEnd = (end + atom - 1) & ~(atom - 1);
            alignedEnd = std::min(alignedEnd, (allocationSize + atom - 1) & ~(atom - 1));
            vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), alignedStart, alignedEnd - alignedStart);
        };

    if (m_memoryPoolHeader.IsDirty)
    {
        flushRange(sizeof(GpuMemoryPoolHeader), static_cast<VkDeviceSize>(m_memoryPoolHeader.Capacity) * m_memoryPoolHeader.SizeOfObject);
        m_memoryPoolHeader.IsDirty = false;
    }

    if (m_isHeaderDirty)
    {
        memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));
        flushRange(0, sizeof(GpuMemoryPoolHeader));
        m_isHeaderDirty = false;
    }

    if (m_isDescriptorSetDirty)
    {
        UpdateDataBufferDescriptorSet(m_memoryPoolBufferIndex, m_memoryPoolDescriptorSetBinding);
        m_isDescriptorSetDirty = false;
    }
}

void MemoryPool::UpdateMemoryPoolHeader(uint32 newCapacity)
{
    MemoryPoolHeader old = m_memoryPoolHeader;
    m_memoryPoolHeader = MemoryPoolHeader{
        .ActiveCount = std::min(old.ActiveCount, newCapacity),
        .Capacity = newCapacity,
        .SizeOfObject = old.SizeOfObject,
        .IsSlotActive = Vector<byte>(newCapacity, 0x00),
        .FreeIndices = {},
        .IsDirty = true
    };

    uint32 copyCount = std::min(old.ActiveCount, newCapacity);
    if (copyCount > 0 && !old.IsSlotActive.empty()) memcpy(m_memoryPoolHeader.IsSlotActive.data(), old.IsSlotActive.data(), copyCount);
    RebuildFreeIndices();

    m_memoryPoolTotalSize = sizeof(GpuMemoryPoolHeader) + static_cast<size_t>(newCapacity) * m_memoryPoolHeader.SizeOfObject;
    m_memoryPoolGpuHeader.ObjectCount = m_memoryPoolHeader.ActiveCount;
    m_memoryPoolGpuHeader.ObjectSize = m_memoryPoolHeader.SizeOfObject;
    m_isHeaderDirty = true;
}

void* MemoryPool::UpdateMemoryPoolObject(uint32 index)
{
    if (index >= m_memoryPoolHeader.Capacity) throw std::out_of_range("MemoryPool index out of range");
    if (!m_memoryPoolHeader.IsSlotActive[index]) throw std::runtime_error("Trying to update inactive slot");

    m_memoryPoolHeader.IsDirty = true;
    return static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(GpuMemoryPoolHeader) + index * m_memoryPoolHeader.SizeOfObject;
}

void MemoryPool::UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding)
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
        .dstSet = m_memoryPoolBindlessDescriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferUpdate,
    };
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
}

void MemoryPool::SwapMemoryPoolElement(uint32 a, uint32 b)
{
    if (a == b) return;
    if (a >= m_memoryPoolHeader.Capacity || b >= m_memoryPoolHeader.Capacity) throw std::out_of_range("slot out of range");

    size_t size = m_memoryPoolHeader.SizeOfObject;
    byte* base = static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(GpuMemoryPoolHeader);

    Vector<byte> temp(size);
    memcpy(temp.data(), base + b * size, size);
    memcpy(base + b * size, base + a * size, size);
    memcpy(base + a * size, temp.data(), size);

    std::swap(m_memoryPoolHeader.IsSlotActive[a], m_memoryPoolHeader.IsSlotActive[b]);

    RebuildFreeIndices();
    m_memoryPoolHeader.IsDirty = true;
}

void MemoryPool::FreeObject(uint32 index)
{
    if (index >= m_memoryPoolHeader.Capacity || !m_memoryPoolHeader.IsSlotActive[index]) return;

    m_memoryPoolHeader.IsSlotActive[index] = 0;
    m_memoryPoolHeader.FreeIndices.push_back(index);
    m_memoryPoolHeader.IsDirty = true;

    while (m_memoryPoolHeader.ActiveCount > 0 && m_memoryPoolHeader.IsSlotActive[m_memoryPoolHeader.ActiveCount - 1] == 0)
    {
        m_memoryPoolHeader.ActiveCount--;
    }
    RefreshGpuHeaderCounts();
}

void MemoryPool::Destroy()
{
    bufferSystem.DestroyBuffer(m_memoryPoolBufferIndex);
    if (m_memoryPoolBindlessPool) vkDestroyDescriptorPool(vulkan.LogicalDevice(), m_memoryPoolBindlessPool, nullptr);
    if (m_memoryPoolBindlessDescriptorSetLayout) vkDestroyDescriptorSetLayout(vulkan.LogicalDevice(), m_memoryPoolBindlessDescriptorSetLayout, nullptr);
}

const Vector<VkDescriptorBufferInfo> MemoryPool::GetBindlessDataBufferDescriptor() const
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex).Buffer(),
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}

const MemoryPoolLoader MemoryPool::GetMemoryPoolInfo() const
{
    return MemoryPoolLoader
    {
        .GlobalBindlessPool = m_memoryPoolBindlessPool,
        .GlobalBindlessDescriptorSet = m_memoryPoolBindlessDescriptorSet,
        .GlobalBindlessDescriptorSetLayout = m_memoryPoolBindlessDescriptorSetLayout
    };
}

