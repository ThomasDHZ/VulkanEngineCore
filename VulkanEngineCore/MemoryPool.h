#pragma once
#include "DLL.h"
#include "Platform.h"
#include <VulkanPipelineLoader.h>
//
//struct GpuMemoryPoolHeader
//{
//	//CPU side: in bytes
//	//GPU side: by index (bytes/4)
//	//GPU data array: by index (bytes/4)
//	uint64 FirstObjectOffset = UINT64_MAX;
//	uint32 ObjectCount		 = UINT32_MAX;
//	uint32 ObjectSize		 = UINT32_MAX;
//};
//
struct MemoryPoolHeader
{
	uint32					ActiveCount   = UINT32_MAX;
	uint32					Capacity	  = UINT32_MAX;
	uint32					SizeOfObject  = UINT32_MAX;
	Vector<byte>			IsSlotActive;         // 0 = inactive, 1 = active
	Vector<uint32>			FreeIndices;
	bool					IsDirty		  = true;
};

//class CORE_DLL_EXPORT MemoryPool
//{
//	private:
//		void*					m_mappedMemoryPoolBufferPtr = nullptr;
//		GpuMemoryPoolHeader		m_memoryPoolGpuHeader;
//		MemoryPoolHeader		m_memoryPoolHeader;
//		size_t					m_memoryPoolTotalSize = SIZE_MAX;
//		uint32					m_memoryPoolBufferIndex = UINT32_MAX;
//		uint32					m_memoryPoolDescriptorSetBinding = UINT32_MAX;
//		VkDescriptorPool		m_memoryPoolBindlessPool = VK_NULL_HANDLE;
//		VkDescriptorSet			m_memoryPoolBindlessDescriptorSet = VK_NULL_HANDLE;
//		VkDescriptorSetLayout	m_memoryPoolBindlessDescriptorSetLayout = VK_NULL_HANDLE;
//		bool					m_isHeaderDirty = true;
//		bool					m_isMemoryPoolDirty = true;
//		bool					m_isDescriptorSetDirty = true;
//
//		void CreateMemoryPool(void* memoryPoolObjectData, size_t memoryPoolObjectCount, uint32 descriptorSetBinding);
//		void CreateBindlessDescriptorSet();
//		void ResizeMemoryPool(uint32 resizeCount);
//		void RefreshGpuHeaderCounts();
//
//	public:
//		MemoryPool();
//		MemoryPool(uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount, uint32 initialCapacity, uint32 descriptorSetBinding);
//		MemoryPool(void* memoryPoolObjectData, uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount, uint32 initialCapacity, uint32 descriptorSetBinding);
//
//		uint32								 AllocateObject();
//		void								 UpdateMemoryPool(size_t baseMemoryPoolSize);
//		void								 UpdateMemoryPoolHeader(uint32 newPoolSize);
//		void*								 UpdateMemoryPoolObject(uint32 index);
//		void								 UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding);
//		void								 SwapMemoryPoolElement(uint32 oldSlotIndex, uint32 newSlotIndex);
//		void								 FreeObject(uint32 index);
//		void								 Destroy();
//		const Vector<VkDescriptorBufferInfo> GetBindlessDataBufferDescriptor() const;
//		const MemoryPoolLoader				 GetMemoryPoolInfo();
//};
//
