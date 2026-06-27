#pragma once
#include <Platform.h>
#include <vk_mem_alloc.h>

struct BufferHandle
{
	uint32 bufferId = 0;
	uint32 generation = 0;
};

inline bool operator==(const BufferHandle& lhs, const BufferHandle& rhs)
{
	return lhs.bufferId == rhs.bufferId && lhs.generation == rhs.generation;
}

inline bool operator!=(const BufferHandle& lhs, const BufferHandle& rhs)
{
	return !(lhs == rhs);
}

namespace std
{
	template<>
	struct hash<BufferHandle>
	{
		size_t operator()(const BufferHandle& h) const noexcept
		{
			return (size_t)h.bufferId ^ ((size_t)h.generation << 32);
		}
	};
}

class DLL_EXPORT VulkanBuffer
{
	friend class BufferSystem;

private:
	BufferHandle        m_handle{};
	VkBuffer            m_buffer = VK_NULL_HANDLE;
	VmaAllocation       m_allocation = VK_NULL_HANDLE;
	VkDeviceSize        m_size = 0;
	VkBufferUsageFlags  m_usage = 0;

	void* m_mappedData = nullptr;
	bool                m_usingStagingBuffer = false;
	bool				m_isPersistentlyMapped = false;

public:
	VulkanBuffer() = default;

	static VulkanBuffer CreateStaticBuffer(BufferHandle bufferId, VkBuffer buffer, VmaAllocation allocation, VkDeviceSize size, VkBufferUsageFlags usage);
	static VulkanBuffer CreateDynamicBuffer(BufferHandle bufferId, VkBuffer buffer, VmaAllocation allocation, VkDeviceSize size, void* mappedData, VkBufferUsageFlags usage);

	[[nodiscard]] BufferHandle        Handle() const noexcept { return m_handle; }
	[[nodiscard]] VkBuffer            Buffer() const noexcept { return m_buffer; }
	[[nodiscard]] VmaAllocation       BufferAllocation() const noexcept { return m_allocation; }
	[[nodiscard]] VkDeviceSize        BufferSize() const noexcept { return m_size; }
	[[nodiscard]] VkBufferUsageFlags  BufferUsage() const noexcept { return m_usage; }
	[[nodiscard]] void*				  BufferMappedData() const noexcept { return m_mappedData; }

	[[nodiscard]] bool                IsValid() const noexcept { return m_buffer != VK_NULL_HANDLE; }
	[[nodiscard]] bool                IsPersistentlyMapped() const noexcept { return m_mappedData != nullptr; }
};