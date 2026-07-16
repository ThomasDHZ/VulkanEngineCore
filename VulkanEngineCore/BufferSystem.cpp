#define VMA_DEBUG_LOG_LEVEL 0   // Reduce spam in RenderDoc
#define VMA_IMPLEMENTATION
#include "BufferSystem.h"
#include <VulkanSystem.h>
#include <vk_mem_alloc.h>
#include <iostream>

BufferSystem& bufferSystem = BufferSystem::Get();

BufferHandle BufferSystem::AllocateBufferId()
{
    BufferHandle handle;
    if (!m_freeList.empty())
    {
        handle.bufferId = m_freeList.back();
        m_freeList.pop_back();
        handle.generation = ++m_generations[handle.bufferId];
    }
    else
    {
        handle.bufferId = m_nextId++;
        if (handle.bufferId >= m_generations.size()) m_generations.resize(handle.bufferId + 64, 0);
        handle.generation = ++m_generations[handle.bufferId];
    }

    return handle;
}

void BufferSystem::FreeBufferId(int id)
{
    if (id > 0 &&
        id < m_generations.size())
    {
        m_freeList.push_back(id);
        m_generations[id]++;
    }
}

VulkanBuffer& BufferSystem::FindVulkanBuffer(uint32 id)
{
    auto it = m_vulkanBufferMap.find(id);
    if (it == m_vulkanBufferMap.end())
    {
        std::cerr << "Error: Buffer ID " << id << " not found!" << std::endl;
        static VulkanBuffer empty{};
        return empty;
    }
    return it->second;
}

const Vector<VulkanBuffer>& BufferSystem::VulkanBufferList()
{
    static Vector<VulkanBuffer> vulkanBufferList;
    vulkanBufferList.clear();
    for (const auto& pair : m_vulkanBufferMap)
    {
        vulkanBufferList.emplace_back(pair.second);
    }
    return vulkanBufferList;
}

void BufferSystem::SetUpVmaAllocation()
{
    VmaVulkanFunctions vulkanFunctions = {
      .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
      .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };

    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = vulkan.PhysicalDevice(),
        .device = vulkan.LogicalDevice(),
        .preferredLargeHeapBlockSize = 64ull << 20,   // 64 MB
        .pVulkanFunctions = &vulkanFunctions,
        .instance = vulkan.InstanceHandle(),
        .vulkanApiVersion = vulkan.ApiVersion(),
    };

    VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &m_vmaAllocator);
    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create VMA allocator: " << result << std::endl;
    }
}

void BufferSystem::CreateStagingBuffer(VkBuffer& outBuffer, VmaAllocation& outAllocation, VkDeviceSize size, const void* data)
{
    if (size == 0)
    {
        outBuffer = VK_NULL_HANDLE;
        outAllocation = VK_NULL_HANDLE;
        return;
    }

    VkBufferCreateInfo bufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    VmaAllocationCreateInfo allocInfo = 
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
               | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocResult{};
    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(VmaAllocatorHandle(), &bufferInfo, &allocInfo, &outBuffer, &outAllocation, &allocResult));
    if (data)
    {
        void* mapped = allocResult.pMappedData;
        if (!mapped) vmaMapMemory(VmaAllocatorHandle(), outAllocation, &mapped);
        memcpy(mapped, data, size);
        vmaFlushAllocation(VmaAllocatorHandle(), outAllocation, 0, size);
        if (!allocResult.pMappedData) vmaUnmapMemory(VmaAllocatorHandle(), outAllocation);
    }
}

uint32 BufferSystem::CreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size,
    VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset)
{
    if (m_vmaAllocator == VK_NULL_HANDLE)
    {
        std::cerr << "[VMA] Allocator is null! Cannot create static buffer." << std::endl;
        return 0;
    }
    if (size == 0) return 0;

    BufferHandle bufferId = AllocateBufferId();
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | shaderUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    VkBuffer dstBuffer = VK_NULL_HANDLE;
    VmaAllocation dstAllocation = VK_NULL_HANDLE;
    VkResult result = vmaCreateBuffer(m_vmaAllocator, &bufferInfo, &allocInfo, &dstBuffer, &dstAllocation, nullptr);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VMA] Failed to create static buffer. Error: " << result << std::endl;
        return 0;
    }

    if (srcData == nullptr)
    {
        m_vulkanBufferMap[bufferId.bufferId] = VulkanBuffer::CreateStaticBuffer(bufferId, dstBuffer, dstAllocation, size, shaderUsageFlags);
        return bufferId.bufferId;
    }

    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo stagingAllocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingAllocOut = {};

    result = vmaCreateBuffer(m_vmaAllocator, &stagingInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocOut);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VMA] Failed to create staging buffer. Error: " << result << std::endl;
        vmaDestroyBuffer(m_vmaAllocator, dstBuffer, dstAllocation);
        return 0;
    }

    void* mappedData = stagingAllocOut.pMappedData;
    bool needUnmap = false;

    if (mappedData == nullptr)
    {
        result = vmaMapMemory(m_vmaAllocator, stagingAllocation, &mappedData);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[VMA] Failed to map staging buffer." << std::endl;
            vmaDestroyBuffer(m_vmaAllocator, stagingBuffer, stagingAllocation);
            vmaDestroyBuffer(m_vmaAllocator, dstBuffer, dstAllocation);
            return 0;
        }
        needUnmap = true;
    }

    memcpy(static_cast<char*>(mappedData) + offset, srcData, size - offset);
    vmaFlushAllocation(m_vmaAllocator, stagingAllocation, offset, size - offset);

    if (needUnmap) vmaUnmapMemory(m_vmaAllocator, stagingAllocation);

    CopyBuffer(&stagingBuffer, &dstBuffer, size - offset, shaderUsageFlags, offset);
    vmaDestroyBuffer(m_vmaAllocator, stagingBuffer, stagingAllocation);

    m_vulkanBufferMap[bufferId.bufferId] = VulkanBuffer::CreateStaticBuffer(bufferId, dstBuffer, dstAllocation, size, shaderUsageFlags);
    return bufferId.bufferId;
}

uint32 BufferSystem::CreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags)
{
    if (m_vmaAllocator == VK_NULL_HANDLE)
    {
        std::cerr << "[VMA] Allocator is null! Cannot create dynamic buffer." << std::endl;
        return 0;
    }
    if (size == 0) return 0;

    BufferHandle bufferId = AllocateBufferId();
    VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VmaAllocationInfo allocOut = {};
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkResult result = vmaCreateBuffer(m_vmaAllocator, &bufferInfo, &allocInfo, &buffer, &allocation, &allocOut);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VMA] Failed to create dynamic buffer. Error: " << result << std::endl;
        return 0;
    }

    void* mappedData = allocOut.pMappedData;
    bool needUnmap = false;
    if (mappedData == nullptr)
    {
        result = vmaMapMemory(m_vmaAllocator, allocation, &mappedData);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[VMA] Failed to map dynamic buffer." << std::endl;
            vmaDestroyBuffer(m_vmaAllocator, buffer, allocation);
            return 0;
        }
        needUnmap = true;
    }

    if (srcData)
    {
        memcpy(mappedData, srcData, size);
        vmaFlushAllocation(m_vmaAllocator, allocation, 0, size);
    }

    m_vulkanBufferMap[bufferId.bufferId] = VulkanBuffer::CreateDynamicBuffer(bufferId, buffer, allocation, size, mappedData, usageFlags);
    return bufferId.bufferId;
}

void BufferSystem::UpdateDynamicBuffer(uint32 bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
    auto it = m_vulkanBufferMap.find(bufferId);
    if (it == m_vulkanBufferMap.end() || !it->second.m_isPersistentlyMapped || !it->second.m_mappedData)
    {
        std::cerr << "[VMA] UpdateDynamicBuffer failed: invalid buffer or not mapped." << std::endl;
        return;
    }

    VulkanBuffer& buffer = it->second;
    memcpy(static_cast<char*>(buffer.m_mappedData) + offset, data, size);
    vmaFlushAllocation(m_vmaAllocator, buffer.m_allocation, offset, size);
}

void BufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkDeviceSize offset)
{
    VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();
    VkBufferCopy copyRegion = {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };
    vkCmdCopyBuffer(cmd, *srcBuffer, *dstBuffer, 1, &copyRegion);

    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = *dstBuffer,
        .offset = offset,
        .size = size
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    vulkan.CommandBuffer().EndSingleUseCommand(cmd);
}

void BufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    FreeBufferId(vulkanBuffer.m_handle.bufferId);
    if (vulkanBuffer.m_buffer != VK_NULL_HANDLE)
    {
        if (vulkanBuffer.m_allocation != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(m_vmaAllocator, vulkanBuffer.m_buffer, vulkanBuffer.m_allocation);
        }
        else
        {
            vkDestroyBuffer(vulkan.LogicalDevice(), vulkanBuffer.m_buffer, nullptr);
        }
        vulkanBuffer.m_buffer = VK_NULL_HANDLE;
        vulkanBuffer.m_allocation = VK_NULL_HANDLE;
    }

    vulkanBuffer.m_mappedData = nullptr;
    vulkanBuffer.m_size = 0;
}

VmaAllocator					          BufferSystem::VmaAllocatorHandle()  const { return m_vmaAllocator; }
UnorderedMap<uint32, VulkanBuffer>        BufferSystem::VulkanBufferMap()     const { return m_vulkanBufferMap; }