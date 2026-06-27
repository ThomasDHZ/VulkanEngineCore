#pragma once
#include <Platform.h>
#include <VulkanSystem.h>
#include "VulkanBuffer.h"

class DLL_EXPORT BufferSystem
{
public:
    static BufferSystem& Get();

private:
    BufferSystem() = default;
    ~BufferSystem() = default;
    BufferSystem(const BufferSystem&) = delete;
    BufferSystem& operator=(const BufferSystem&) = delete;

private:
    uint32                                            m_nextId = 1;
    Vector<uint32>                                    m_freeList;
    Vector<uint32>                                    m_generations;
    VmaAllocator                                      m_vmaAllocator;
    UnorderedMap<BufferHandle, VulkanBuffer>          m_vulkanBufferMap;

    BufferHandle                                      AllocateBufferId();
    void                                              FreeBufferId(int id);

public:

    template <typename T>
    uint32 CreateVulkanBuffer(T& bufferData, VkBufferUsageFlags shaderUsageFlags, bool usingStagingBuffer)
    {
        if (usingStagingBuffer)
        {
            return CreateStaticVulkanBuffer(static_cast<void*>(&bufferData), sizeof(T), shaderUsageFlags);
        }
        else
        {
            return CreateDynamicBuffer(static_cast<void*>(&bufferData), sizeof(T), shaderUsageFlags);
        }
    }

    template <typename T>
    uint32 CreateVulkanBuffer(Vector<T>& bufferData, VkBufferUsageFlags shaderUsageFlags, bool usingStagingBuffer)
    {
        VkDeviceSize bufferSize = sizeof(T) * bufferData.size();
        if (bufferData.empty())
        {
            bufferSize = sizeof(T) * 64;
        }

        if (usingStagingBuffer)
        {
            return CreateStaticVulkanBuffer(bufferData.data(), bufferSize, shaderUsageFlags);
        }
        else
        {
            return CreateDynamicBuffer(bufferData.data(), bufferSize, shaderUsageFlags);
        }
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, T& bufferData)
    {
        UpdateDynamicBuffer(bufferId, static_cast<void*>(&bufferData), sizeof(T));
    }

    template <typename T>
    void UpdateBufferMemory(uint32 bufferId, Vector<T>& bufferData)
    {
        UpdateDynamicBuffer(bufferId, bufferData.data(), sizeof(T) * bufferData.size());
    }

    template <typename T>
    Vector<T> CheckBufferMemory(uint32 vulkanBufferId)
    {
        VulkanBuffer& vulkanBuffer = FindVulkanBuffer(vulkanBufferId);

        Vector<T> DataList;
        /*     size_t dataListSize = vulkanBuffer.BufferSize / sizeof(T);

             void* data = MapBufferMemory(vulkanBuffer.BufferMemory, vulkanBuffer.BufferSize, &vulkanBuffer.IsMapped);
             if (data == nullptr)
             {
                 std::cerr << "Failed to map buffer memory\n";
                 return DataList;
             }

             char* newPtr = static_cast<char*>(data);
             for (size_t x = 0; x < dataListSize; ++x)
             {
                 DataList.emplace_back(*reinterpret_cast<T*>(newPtr));
                 newPtr += sizeof(T);
             }
             UnmapBufferMemory(vulkanBuffer.BufferMemory, &vulkanBuffer.IsMapped);*/

        return DataList;
    }

    void                                                    SetUpVmaAllocation();
    uint32                                                  CreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset = 0);
    uint32                                                  CreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags);
    void                                                    UpdateDynamicBuffer(BufferHandle bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    void                                                    CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset = 0);
    void                                                    DestroyBuffer(VulkanBuffer& vulkanBuffer);
    VulkanBuffer&                                           FindVulkanBuffer(BufferHandle id);
    const Vector<VulkanBuffer>&                             VulkanBufferList();

    [[nodiscard]] VmaAllocator			                    VmaAllocatorHandle()	    const;
    [[nodiscard]] UnorderedMap<BufferHandle, VulkanBuffer>  VulkanBufferMap()         const;
};
extern DLL_EXPORT BufferSystem& bufferSystem;
inline BufferSystem& BufferSystem::Get()
{
    static BufferSystem instance;
    return instance;
}