//#include "MemoryPool.h"
//#include <BufferSystem.h>
//
//
//MemoryPool::MemoryPool()
//{
//}
//
//MemoryPool::MemoryPool(uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount)
//{
//	m_memoryPoolHeader = MemoryPoolHeader
//	{
//		.ActiveCount = 0,
//		.Capacity = memoryPoolObjectCount,
//		.Size = memoryPoolObjectSizePerObject,
//		.IsSlotActive = Vector<byte>(memoryPoolObjectCount, 0x00),
//		.FreeIndices = Vector<uint32>(),
//		.IsDirty = true
//	};
//	CreateMemoryPool(nullptr, memoryPoolObjectSizePerObject, memoryPoolObjectCount);
//}
//
//MemoryPool::MemoryPool(void* memoryPoolObjectData, uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount)
//{
//	m_memoryPoolHeader = MemoryPoolHeader
//	{
//		.ActiveCount = 0,
//		.Capacity = memoryPoolObjectCount,
//		.Size = memoryPoolObjectSizePerObject,
//		.IsSlotActive = Vector<byte>(memoryPoolObjectCount, 0x00),
//		.FreeIndices = Vector<uint32>(),
//		.IsDirty = true
//	};
//	CreateMemoryPool(memoryPoolObjectData, memoryPoolObjectSizePerObject, memoryPoolObjectCount);
//}
//
//uint32 MemoryPool::AllocateObject()
//{
//	MemoryPoolSubBufferHeader& subPoolHeader = MemorySubPoolHeader[memoryPoolToUpdate];
//
//	if (!subPoolHeader.FreeIndices.empty())
//	{
//		uint32 index = subPoolHeader.FreeIndices.back();
//		subPoolHeader.FreeIndices.pop_back();
//		subPoolHeader.IsSlotActive[index] = 1;
//
//		if (index + 1 > subPoolHeader.ActiveCount) subPoolHeader.ActiveCount = index + 1;
//		subPoolHeader.IsDirty = true;
//		RefreshGpuHeaderCounts();
//		return index;
//	}
//
//	if (subPoolHeader.ActiveCount == subPoolHeader.Capacity)
//		ResizeMemoryPool(memoryPoolToUpdate, subPoolHeader.Capacity * 2);
//
//	uint32 index = subPoolHeader.ActiveCount++;
//	subPoolHeader.IsSlotActive[index] = 0x01;
//	subPoolHeader.IsDirty = true;
//	RefreshGpuHeaderCounts();
//	return index;
//}
//
//void MemoryPool::UpdateMemoryPool(void* memoryPoolObjectPtr, uint32 sizeOfMemoryPoolObject, uint32 index)
//{
//	if (index >= meshSubPool.Capacity) throw std::out_of_range("Mesh index out of range: " + std::to_string(index) + " >= " + std::to_string(meshSubPool.Capacity));
//	if (index >= meshSubPool.IsSlotActive.size() || !meshSubPool.IsSlotActive[index]) throw std::runtime_error("Mesh slot inactive at index " + std::to_string(index));
//
//	uint32 offset = index * sizeOfMemoryPoolObject;
//	return *reinterpret_cast<MeshPropertiesStruct*>(static_cast<byte*>(MappedBufferPtr) + offset);
//	meshSubPool.IsDirty = true;
//}
//
//void MemoryPool::CreateMemoryPool(void* memoryPoolObjectData, size_t memoryPoolObjectSizePerObject, size_t memoryPoolObjectCount)
//{
//	m_memoryPoolTotalSize = memoryPoolObjectSizePerObject * memoryPoolObjectCount;
//	m_memoryPoolBufferIndex = bufferSystem.CreateDynamicBuffer(memoryPoolObjectData, m_memoryPoolTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//
//	VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex);
//	m_mappedMemoryPoolBufferPtr = buffer.BufferMappedData();
//	memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolHeader, sizeof(m_memoryPoolHeaderSize));
//	vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), 0, m_memoryPoolTotalSize);
//}
//
//void MemoryPool::ResizeMemoryPool(uint32 resizeCount)
//{
//	void* oldMappedPtr = m_mappedMemoryPoolBufferPtr;
//	uint32 oldBufferId = m_memoryPoolBufferIndex;
//
//	size_t newTotalSize = m_memoryPoolTotalSize;
//	uint32 newBufferId = bufferSystem.CreateDynamicBuffer(nullptr, newTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//	VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
//	m_mappedMemoryPoolBufferPtr = newBuf.BufferMappedData();
//
//	std::memset(static_cast<byte*>(m_mappedMemoryPoolBufferPtr), 0xFF, newTotalSize);
//	memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolHeader, m_memoryPoolHeaderSize);
//	vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), newBuf.BufferAllocation(), 0, newTotalSize);
//
//	byte* src = static_cast<byte*>(oldMappedPtr);
//	byte* dst = static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(m_memoryPoolHeaderSize);
//	memcpy(dst, src, m_memoryPoolHeader.ActiveCount * m_memoryPoolHeader.Size);
//
//	if (oldBufferId != UINT32_MAX)
//	{
//		bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(oldBufferId));
//	}
//
//	m_memoryPoolBufferIndex = newBufferId;
//}
