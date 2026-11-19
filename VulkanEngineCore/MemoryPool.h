#pragma once
#include "DLL.h"
#include "Platform.h"
#include <VulkanPipelineLoader.h>
#include <VulkanTexture.h>

struct GpuMemoryPoolHeader
{
    uint64 FirstObjectOffset = sizeof(GpuMemoryPoolHeader);
    uint32 ObjectCount = 0;
    uint32 ObjectSize = 0;
};

struct MemoryPoolHeader
{
    uint32          ActiveCount = 0;
    uint32          Capacity = 0;
    uint32          SizeOfObject = 0;
    Vector<byte>    IsSlotActive;   // 0 = inactive, 1 = active
    Vector<uint32>  FreeIndices;
    bool            IsDirty = true;
};

struct TextureMetadataHeader
{
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 0;
    uint32 MipLevels = 1;
    uint32 LayerCount = 1;  // 1 = 2D, 6 = Cubemap
    uint32 Format = 1;
    uint32 Type = 0;       // 0 = 2D, 1 = Cube
    uint32 ArrayIndex = 0; // Index into the correct sampler array
};

class CORE_DLL_EXPORT MemoryPool
{
private:
    void*                                m_mappedMemoryPoolBufferPtr = nullptr;
    GpuMemoryPoolHeader                  m_memoryPoolGpuHeader{};
    MemoryPoolHeader                     m_memoryPoolHeader{};
    size_t                               m_memoryPoolTotalSize = 0;
    uint32                               m_memoryPoolBufferIndex = UINT32_MAX;
    uint32                               m_memoryPoolDescriptorSetBinding = UINT32_MAX;
    VkDescriptorPool                     m_memoryPoolBindlessPool = VK_NULL_HANDLE;
    VkDescriptorSet                      m_memoryPoolBindlessDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout                m_memoryPoolBindlessDescriptorSetLayout = VK_NULL_HANDLE;
    bool                                 m_isHeaderDirty = true;
    bool                                 m_isDescriptorSetDirty = true;

    void                                 CreateMemoryPool(void* data, size_t count, uint32 binding);
    void                                 CreateBindlessDescriptorSet();
    void                                 UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding);
    void								 UpdateTextureDescriptorSet(uint32 textureGpuBufferIndex, VulkanTexture& texture);
    void                                 UpdateMemoryPoolHeader(uint32 newCapacity);
    void                                 ResizeMemoryPool(uint32 newCapacity);
    void                                 RefreshGpuHeaderCounts();
    void                                 RebuildFreeIndices();

public:

    MemoryPool();
    MemoryPool(uint32 objectSize, uint32 initialCapacity, uint32 descriptorSetBinding);
    MemoryPool(void* initialData, uint32 objectSize, uint32 objectCount, uint32 initialCapacity, uint32 descriptorSetBinding);
    ~MemoryPool();

    template<typename T>
    MemoryPool(Vector<T>& objectList, uint32 initialCapacity, uint32 descriptorSetBinding)
    {
        m_memoryPoolHeader = MemoryPoolHeader{
            .ActiveCount = 0,
            .Capacity = initialCapacity,
            .SizeOfObject = sizeof(T),
            .IsSlotActive = Vector<byte>(initialCapacity, 0x00),
            .FreeIndices = {},
            .IsDirty = true
        };
        CreateMemoryPool(objectList.data(), objectList.size(), descriptorSetBinding);
    }

    template<typename T>
    uint32 Allocate()
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        assert(sizeof(T) == m_memoryPoolHeader.SizeOfObject && "Size mismatch");
        return AllocateObject();
    }

    template<typename T>
    T& UpdateMemoryPoolObject(uint32 index)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        assert(sizeof(T) == m_memoryPoolHeader.SizeOfObject && "Size mismatch");
        return *static_cast<T*>(UpdateMemoryPoolObject(index));
    }

    template<typename T>
    void UpdateMemoryPoolObject(uint32 index, T& updateObject)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        assert(sizeof(T) == m_memoryPoolHeader.SizeOfObject && "Size mismatch");
        *static_cast<T*>(UpdateMemoryPoolObject(index)) = updateObject;
    }

    template<typename T>
    Vector<T> ObjectList() const
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        assert(sizeof(T) == m_memoryPoolHeader.SizeOfObject);

        Vector<T> result(m_memoryPoolHeader.ActiveCount);
        if (m_memoryPoolHeader.ActiveCount == 0) return result;

        const byte* src = static_cast<const byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(GpuMemoryPoolHeader);
        std::memcpy(result.data(), src, m_memoryPoolHeader.ActiveCount * sizeof(T));
        return result;
    }

    uint32                               AllocateObject();
    uint32                               AddTextureToMemoryPool(VulkanTexture& texture);
    void                                 UpdateMemoryPool();
    void*                                UpdateMemoryPoolObject(uint32 index);
    void                                 SwapMemoryPoolElement(uint32 a, uint32 b);
    void                                 FreeObject(uint32 index);
    const Vector<VkDescriptorBufferInfo> GetBindlessDataBufferDescriptor() const;
    const MemoryPoolLoader               GetMemoryPoolInfo() const;
    void                                 Destroy();

    uint32                               ActiveCount() const;
    uint32                               Capacity()    const;
};