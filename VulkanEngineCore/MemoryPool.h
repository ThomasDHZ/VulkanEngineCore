//#pragma once
//#include "DLL.h"
//#include "Platform.h"
//
//struct MemoryPoolHeader
//{
//	uint32					ActiveCount = UINT32_MAX;
//	size_t					Offset = UINT32_MAX;
//	uint32					Capacity = UINT32_MAX;
//	uint32					Size = UINT32_MAX;
//	Vector<byte>			IsSlotActive;         // 0 = inactive, 1 = active
//	Vector<uint32>			FreeIndices;
//	bool					IsDirty = true;
//};
//
//class MemoryPool
//{
//	private:
//		static constexpr size_t m_memoryPoolHeaderSize = sizeof(MemoryPoolHeader);
//
//		void*					m_mappedMemoryPoolBufferPtr = nullptr;
//		uint					m_memoryPoolBufferIndex = UINT32_MAX;
//		size_t					m_memoryPoolCapacity = SIZE_MAX;
//		size_t					m_memoryPoolTotalSize = SIZE_MAX;
//		MemoryPoolHeader		m_memoryPoolHeader;
//		bool					m_isMemoryPoolDirty = true;
//
//		void CreateMemoryPool(void* memoryPoolObjectData, size_t memoryPoolObjectSizePerObject, size_t memoryPoolObjectCount);
//		void ResizeMemoryPool(uint32 resizeCount);
//	public:
//		MemoryPool();
//		MemoryPool(uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount);
//		MemoryPool(void* memoryPoolObjectData, uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount);
//
//		template <typename T>
//		MemoryPool(uint32 initialCapacity)
//		{
//			m_memoryPoolHeader = MemoryPoolHeader
//			{
//				.ActiveCount = 0,
//				.Capacity = initialCapacity,
//				.Size = sizeof(T),
//				.IsSlotActive = Vector<byte>(initialCapacity, 0x00),
//				.FreeIndices = Vector<uint32>(),
//				.IsDirty = true
//			};
//			CreateMemoryPool(memoryPoolObjectData, memoryPoolObjectSizePerObject, initialCapacity);
//		}
//
//		template <typename T>
//		MemoryPool(Vector<T>& objectList, uint32 initialCapacity)
//		{
//			m_memoryPoolHeader = MemoryPoolHeader
//			{
//				.ActiveCount = 0,
//				.Capacity = initialCapacity,
//				.Size = sizeof(T),
//				.IsSlotActive = Vector<byte>(initialCapacity, 0x00),
//				.FreeIndices = Vector<uint32>(),
//				.IsDirty = true
//			};
//			CreateMemoryPool(memoryPoolObjectData, memoryPoolObjectSizePerObject, initialCapacity);
//		}
//
//		template <typename T>
//		T& UpdateMemoryPool(uint32 index)
//		{
//
//		}
//
//		uint32 AllocateObject();
//		void UpdateMemoryPool(void* memoryPoolObjectPtr, uint32 sizeOfMemoryPoolObject, uint32 index);
//};
//
