#include "VulkanBuffer.h"

VulkanBuffer VulkanBuffer::CreateStaticBuffer(BufferHandle bufferId, VkBuffer buffer, VmaAllocation allocation, VkDeviceSize size, VkBufferUsageFlags usage)
{
    VulkanBuffer staticBuffer;
    staticBuffer.m_handle = bufferId;
    staticBuffer.m_buffer = buffer;
    staticBuffer.m_allocation = allocation;
    staticBuffer.m_size = size;
    staticBuffer.m_usage = usage;
    staticBuffer.m_usingStagingBuffer = true;
    return staticBuffer;
}

VulkanBuffer VulkanBuffer::CreateDynamicBuffer(BufferHandle bufferId, VkBuffer buffer, VmaAllocation allocation, VkDeviceSize size, void* mappedData, VkBufferUsageFlags usage)
{
    VulkanBuffer dynamicBuffer;
    dynamicBuffer.m_handle = bufferId;
    dynamicBuffer.m_buffer = buffer;
    dynamicBuffer.m_allocation = allocation;
    dynamicBuffer.m_size = size;
    dynamicBuffer.m_usage = usage;
    dynamicBuffer.m_mappedData = mappedData;
    dynamicBuffer.m_isPersistentlyMapped = true;
    return dynamicBuffer;
}
