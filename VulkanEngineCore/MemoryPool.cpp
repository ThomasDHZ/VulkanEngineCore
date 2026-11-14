//#include "MemoryPool.h"
//#include <BufferSystem.h>
//#include <MemorySystem.h>
//
//MemoryPool::MemoryPool()
//{
//}
//
//MemoryPool::MemoryPool(uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount, uint32 initialCapacity, uint32 descriptorSetBinding)
//{
//	m_memoryPoolHeader = MemoryPoolHeader{
//			.ActiveCount = 0,
//			.Capacity = initialCapacity,
//			.SizeOfObject = memoryPoolObjectSizePerObject, 
//			.IsSlotActive = Vector<byte>(initialCapacity, 0x00),
//			.FreeIndices = Vector<uint32>(),
//			.IsDirty = true
//	};
//	CreateMemoryPool(nullptr, memoryPoolObjectCount, descriptorSetBinding);
//}
//
//MemoryPool::MemoryPool(void* memoryPoolObjectData, uint32 memoryPoolObjectSizePerObject, uint32 memoryPoolObjectCount, uint32 initialCapacity, uint32 descriptorSetBinding)
//{
//	m_memoryPoolHeader = MemoryPoolHeader{
//				.ActiveCount = 0,
//				.Capacity = initialCapacity,
//				.SizeOfObject = memoryPoolObjectSizePerObject,
//				.IsSlotActive = Vector<byte>(initialCapacity, 0x00),
//				.FreeIndices = Vector<uint32>(),
//				.IsDirty = true
//	};
//	CreateMemoryPool(memoryPoolObjectData, memoryPoolObjectCount, descriptorSetBinding);
//}
//
//void MemoryPool::CreateMemoryPool(void* memoryPoolObjectData, size_t memoryPoolObjectCount, uint32 descriptorSetBinding)
//{
//	m_memoryPoolTotalSize = sizeof(GpuMemoryPoolHeader) + static_cast<size_t>(m_memoryPoolHeader.Capacity) * m_memoryPoolHeader.SizeOfObject;
//	m_memoryPoolGpuHeader = GpuMemoryPoolHeader
//	{
//		.FirstObjectOffset = sizeof(GpuMemoryPoolHeader),
//		.ObjectCount = 0,
//		.ObjectSize = m_memoryPoolHeader.SizeOfObject
//	};
//
//	m_memoryPoolDescriptorSetBinding = descriptorSetBinding;
//	m_memoryPoolBufferIndex = bufferSystem.CreateDynamicBuffer(nullptr, m_memoryPoolTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//	VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex);
//	m_mappedMemoryPoolBufferPtr = buffer.BufferMappedData();
//
//	memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));
//	vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), 0, m_memoryPoolTotalSize);
//	CreateBindlessDescriptorSet();
//}
//
//uint32 MemoryPool::AllocateObject()
//{
//	if (!m_memoryPoolHeader.FreeIndices.empty())
//	{
//        uint32 index = m_memoryPoolHeader.FreeIndices.back();
//		m_memoryPoolHeader.FreeIndices.pop_back();
//		m_memoryPoolHeader.IsSlotActive[index] = 1;
//
//        if (index + 1 > m_memoryPoolHeader.ActiveCount) m_memoryPoolHeader.ActiveCount = index + 1;
//		m_memoryPoolHeader.IsDirty = true;
//        RefreshGpuHeaderCounts();
//        return index;
//	}
//
//	if (m_memoryPoolHeader.ActiveCount == m_memoryPoolHeader.Capacity) ResizeMemoryPool(m_memoryPoolHeader.Capacity * 2);
//
//	uint32 index = m_memoryPoolHeader.ActiveCount++;
//	m_memoryPoolHeader.IsSlotActive[index] = 0x01;
//	m_memoryPoolHeader.IsDirty = true;
//	RefreshGpuHeaderCounts();
//	return index;
//}
//
//void MemoryPool::UpdateMemoryPoolHeader(uint32 newPoolSize)
//{
//	MemoryPoolHeader old = m_memoryPoolHeader; 
//	m_memoryPoolHeader = MemoryPoolHeader{
//		.ActiveCount = old.ActiveCount,
//		.Capacity = newPoolSize,
//		.SizeOfObject = old.SizeOfObject,
//		.IsSlotActive = Vector<byte>(newPoolSize, 0x00),
//		.FreeIndices = old.FreeIndices,
//		.IsDirty = true
//	};
//
//	const uint32 bytesToCopy = std::min(old.ActiveCount, newPoolSize);
//	if (bytesToCopy > 0 && !old.IsSlotActive.empty())
//	{
//		memcpy(m_memoryPoolHeader.IsSlotActive.data(), old.IsSlotActive.data(), bytesToCopy);
//	}
//
//	m_memoryPoolTotalSize = sizeof(GpuMemoryPoolHeader) + static_cast<size_t>(newPoolSize) * m_memoryPoolHeader.SizeOfObject;
//	m_memoryPoolGpuHeader = GpuMemoryPoolHeader{
//		.FirstObjectOffset = sizeof(GpuMemoryPoolHeader),
//		.ObjectCount = m_memoryPoolHeader.ActiveCount,
//		.ObjectSize = m_memoryPoolHeader.SizeOfObject
//	};
//}
//
//void MemoryPool::UpdateMemoryPool(size_t baseMemoryPoolSize)
//{
//	if (!m_mappedMemoryPoolBufferPtr) return;
//
//	VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex);
//	const VkDeviceSize atom = vulkan.Device().GetPhysicalDeviceProperties(vulkan.PhysicalDevice()).limits.nonCoherentAtomSize;
//	const VkDeviceSize allocationSize = sizeof(GpuMemoryPoolHeader) + (static_cast<VkDeviceSize>(baseMemoryPoolSize) * m_memoryPoolHeader.Capacity);
//	auto flushRange = [&](VkDeviceSize start, VkDeviceSize len)
//		{
//			if (len == 0 || start >= allocationSize) return;
//
//			VkDeviceSize end = std::min(start + len, allocationSize);
//			VkDeviceSize alignedStart = start & ~(atom - 1);
//			VkDeviceSize alignedEnd = (end + atom - 1) & ~(atom - 1);
//			alignedEnd = std::min(alignedEnd, (allocationSize + atom - 1) & ~(atom - 1));
//			vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), alignedStart, alignedEnd - alignedStart);
//		};
//
//	if (m_memoryPoolHeader.IsDirty)
//	{
//		flushRange(sizeof(GpuMemoryPoolHeader), static_cast<VkDeviceSize>(m_memoryPoolHeader.Capacity) * m_memoryPoolHeader.SizeOfObject);
//		m_memoryPoolHeader.IsDirty = false;
//	}
//
//	if (m_isHeaderDirty)
//	{
//		memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));
//		flushRange(0, sizeof(GpuMemoryPoolHeader));
//		m_isHeaderDirty = false;
//	}
//
//	if (m_isDescriptorSetDirty)
//	{
//		UpdateDataBufferDescriptorSet(m_memoryPoolBufferIndex, m_memoryPoolDescriptorSetBinding);
//		m_isDescriptorSetDirty = false;
//	}
//}
//
//void MemoryPool::ResizeMemoryPool(uint32 resizeCount)
//{
//	void* oldMappedPtr = m_mappedMemoryPoolBufferPtr;
//	uint32 oldBufferId = m_memoryPoolBufferIndex;
//	auto oldSubHeaders = m_memoryPoolHeader;
//
//	UpdateMemoryPoolHeader(resizeCount);
//	size_t newTotalSize = m_memoryPoolTotalSize;
//	uint32 newBufferId = bufferSystem.CreateDynamicBuffer(nullptr, newTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
//	VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
//	m_mappedMemoryPoolBufferPtr = newBuf.BufferMappedData();
//
//	std::memset(static_cast<byte*>(m_mappedMemoryPoolBufferPtr), 0xFF, newTotalSize);
//	memcpy(m_mappedMemoryPoolBufferPtr, &m_memoryPoolGpuHeader, sizeof(GpuMemoryPoolHeader));
//
//	const MemoryPoolHeader& oldMemoryPoolHeader = m_memoryPoolHeader;
//	size_t bytesToCopy = m_memoryPoolHeader.ActiveCount * m_memoryPoolHeader.SizeOfObject;
//	if (bytesToCopy > 0)
//	{
//		byte* dst = static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + sizeof(GpuMemoryPoolHeader);
//		byte* src = static_cast<byte*>(oldMappedPtr) + sizeof(GpuMemoryPoolHeader);
//		memcpy(dst, src, bytesToCopy);
//	}
//
//	vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), newBuf.BufferAllocation(), 0, newTotalSize);
//	if (oldBufferId != UINT32_MAX)
//	{
//		bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(oldBufferId));
//	}
//
//	m_memoryPoolBufferIndex = newBufferId;
//	m_isDescriptorSetDirty = true;
//	m_isHeaderDirty = true;
//}
//
//void MemoryPool::RefreshGpuHeaderCounts()
//{
//	m_memoryPoolGpuHeader.ObjectCount = m_memoryPoolHeader.ActiveCount;
//	m_isHeaderDirty = true;
//}
//
//void* MemoryPool::UpdateMemoryPoolObject(uint32 index)
//{
//	if (index >= m_memoryPoolHeader.Capacity) throw std::out_of_range("Memory Pool index out of range: " + std::to_string(index));
//	if (index >= m_memoryPoolHeader.IsSlotActive.size() || !m_memoryPoolHeader.IsSlotActive[index]) throw std::runtime_error("Mesh slot inactive at index " + std::to_string(index));
//
//	m_memoryPoolHeader.IsDirty = true;
//	uint32 offset = sizeof(GpuMemoryPoolHeader) + (index * m_memoryPoolHeader.SizeOfObject);
//	return static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + offset;
//}
//
//void MemoryPool::CreateBindlessDescriptorSet()
//{
//	VkDescriptorBufferInfo  bindlessDataBuffer = VkDescriptorBufferInfo
//	{
//		.buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex).Buffer(),
//		.offset = 0,
//		.range = VK_WHOLE_SIZE
//	};
//
//	Vector<VkDescriptorPoolSize> poolSizes = {
//		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512}
//	};
//
//	Vector<VkDescriptorSetLayoutBinding> bindings =
//	{
//		{ m_memoryPoolDescriptorSetBinding   , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1,                             VK_SHADER_STAGE_ALL},
//	};
//
//	Vector<VkDescriptorBindingFlags> flags =
//	{
//		VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT }
//	};
//
//	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
//	{
//		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
//		.bindingCount = static_cast<uint32>(flags.size()),
//		.pBindingFlags = flags.data()
//	};
//
//	VkDescriptorPoolCreateInfo poolInfo =
//	{
//		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
//		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
//		.maxSets = 64,
//		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
//		.pPoolSizes = poolSizes.data()
//	};
//	VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkan.LogicalDevice(), &poolInfo, nullptr, &m_memoryPoolBindlessPool));
//
//	VkDescriptorSetLayoutCreateInfo layoutInfo =
//	{
//		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//		.pNext = &flagsInfo,
//		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
//		.bindingCount = static_cast<uint32_t>(bindings.size()),
//		.pBindings = bindings.data()
//	};
//	VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkan.LogicalDevice(), &layoutInfo, nullptr, &m_memoryPoolBindlessDescriptorSetLayout));
//
//	VkDescriptorSetAllocateInfo allocInfo =
//	{
//		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//		.descriptorPool = m_memoryPoolBindlessPool,
//		.descriptorSetCount = 1,
//		.pSetLayouts = &m_memoryPoolBindlessDescriptorSetLayout
//	};
//	VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkan.LogicalDevice(), &allocInfo, &m_memoryPoolBindlessDescriptorSet));
//
//	Vector<VkWriteDescriptorSet> writeDescriptorSetList = Vector<VkWriteDescriptorSet>
//	{
//		VkWriteDescriptorSet
//			{
//				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//				.dstSet = m_memoryPoolBindlessDescriptorSet,
//				.dstBinding = m_memoryPoolDescriptorSetBinding,
//				.dstArrayElement = 0,
//				.descriptorCount = 1,
//				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//				.pBufferInfo = &bindlessDataBuffer,
//			}
//	};
//	vkUpdateDescriptorSets(vulkan.LogicalDevice(), static_cast<uint32>(writeDescriptorSetList.size()), writeDescriptorSetList.data(), 0, nullptr);
//
//}
//
//void MemoryPool::UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding)
//{
//	VkDescriptorBufferInfo bufferUpdate = VkDescriptorBufferInfo
//	{
//		.buffer = bufferSystem.FindVulkanBuffer(vulkanGpuBufferIndex).Buffer(),
//		.offset = 0,
//		.range = VK_WHOLE_SIZE
//	};
//
//	VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
//	{
//		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//		.dstSet = m_memoryPoolBindlessDescriptorSet,
//		.dstBinding = binding,
//		.dstArrayElement = 0,
//		.descriptorCount = 1,
//		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//		.pBufferInfo = &bufferUpdate,
//	};
//	vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
//}
//
//void MemoryPool::SwapMemoryPoolElement(uint32 oldSlotIndex, uint32 newSlotIndex)
//{
//	if (oldSlotIndex == newSlotIndex) return;
//	if (oldSlotIndex >= m_memoryPoolHeader.Capacity || newSlotIndex >= m_memoryPoolHeader.Capacity) throw std::out_of_range("slot out of range");
//
//	uint32 oldOffset = sizeof(GpuMemoryPoolHeader) + (oldSlotIndex * m_memoryPoolHeader.SizeOfObject);
//	uint32 newOffset = sizeof(GpuMemoryPoolHeader) + (newSlotIndex * m_memoryPoolHeader.SizeOfObject);
//
//	void* temp = memorySystem.AddPtrBuffer<void*>(m_memoryPoolHeader.SizeOfObject, __FILE__, __LINE__, __func__);
//	std::memcpy(temp, static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + newOffset, m_memoryPoolHeader.SizeOfObject);
//	std::memcpy(static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + newOffset, static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + oldOffset, m_memoryPoolHeader.SizeOfObject);
//	std::memcpy(static_cast<byte*>(m_mappedMemoryPoolBufferPtr) + oldOffset, temp, m_memoryPoolHeader.SizeOfObject);
//	memorySystem.DeletePtr(temp);
//
//	std::swap(m_memoryPoolHeader.IsSlotActive[oldSlotIndex], m_memoryPoolHeader.IsSlotActive[newSlotIndex]);
//	if (m_memoryPoolHeader.IsSlotActive[newSlotIndex] == 0x01)
//	{
//
//	}
//	// If the free list contains either index, it must be updated too
//	// (or simply rebuild FreeIndices from IsSlotActive if you prefer simplicity)
//
//	m_memoryPoolHeader.IsDirty = true;
//}
//
//void MemoryPool::FreeObject(uint32 index)
//{
//	if (index >= m_memoryPoolHeader.Capacity || index >= m_memoryPoolHeader.IsSlotActive.size() || !m_memoryPoolHeader.IsSlotActive[index]) return;
//
//	m_memoryPoolHeader.IsSlotActive[index] = 0x00;
//	m_memoryPoolHeader.FreeIndices.push_back(index);
//	m_memoryPoolHeader.IsDirty = true;
//
//	while (m_memoryPoolHeader.ActiveCount > 0 && m_memoryPoolHeader.IsSlotActive[m_memoryPoolHeader.ActiveCount - 1] == 0)
//	{
//		m_memoryPoolHeader.ActiveCount--;
//	}
//	RefreshGpuHeaderCounts();
//}
//
//void MemoryPool::Destroy()
//{
//	bufferSystem.DestroyBuffer(m_memoryPoolBufferIndex);
//	if(m_memoryPoolBindlessPool) vkDestroyDescriptorPool(vulkan.LogicalDevice(), m_memoryPoolBindlessPool, nullptr);
//	if (m_memoryPoolBindlessDescriptorSetLayout) vkDestroyDescriptorSetLayout(vulkan.LogicalDevice(), m_memoryPoolBindlessDescriptorSetLayout, nullptr);
//}
//
//const Vector<VkDescriptorBufferInfo> MemoryPool::GetBindlessDataBufferDescriptor() const
//{
//	return Vector<VkDescriptorBufferInfo>
//	{
//		VkDescriptorBufferInfo
//		{
//			.buffer = bufferSystem.FindVulkanBuffer(m_memoryPoolBufferIndex).Buffer(),
//			.offset = 0,
//			.range = VK_WHOLE_SIZE
//		}
//	};
//}
//
//const MemoryPoolLoader MemoryPool::GetMemoryPoolInfo()
//{
//	return MemoryPoolLoader
//	{
//		.GlobalBindlessPool = m_memoryPoolBindlessPool,
//		.GlobalBindlessDescriptorSet = m_memoryPoolBindlessDescriptorSet,
//		.GlobalBindlessDescriptorSetLayout = m_memoryPoolBindlessDescriptorSetLayout
//	};
//}