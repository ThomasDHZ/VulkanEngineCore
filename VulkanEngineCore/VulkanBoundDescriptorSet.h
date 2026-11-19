#pragma once
#include "DLL.h"
#include <Platform.h>
#include "VulkanTexture.h"

struct VulkanDescriptorSet
{
	uint32 Binding;
	VkDescriptorType DescriptorType;
	uint32 DescriptorCount;
	uint32 DescriptorPoolCount;
	VkShaderStageFlags StageFlags;
	VkDescriptorBindingFlags DescriptorBindingFlags = 0;
	VkDescriptorSetLayoutBinding* pBindings = nullptr;
	VkSampler* pImmutableSamplers = nullptr;
};

class CORE_DLL_EXPORT VulkanBoundDescriptorSet
{
private:
	uint32								 m_inputAttachmentCount = 0;
	Vector<VulkanDescriptorSet>			 m_vulkanDescriptorSetList = Vector<VulkanDescriptorSet>();
	VkDescriptorSet						 m_descriptorSetList = VK_NULL_HANDLE;
	VkDescriptorSetLayout				 m_descriptorSetLayoutList = VK_NULL_HANDLE;
	VkDescriptorPool					 m_descriptorPool = VK_NULL_HANDLE;

	Vector<VkDescriptorPoolSize>		 GetDescriptorPoolCounts();
	Vector<VkDescriptorBindingFlags>	 GetDescriptorPoolFlags();
	Vector<VkDescriptorSetLayoutBinding> GetDescriptorSetLayoutCreateInfo();
public:
	VulkanBoundDescriptorSet();
	VulkanBoundDescriptorSet(uint32 inputAttachmentCount);
	~VulkanBoundDescriptorSet();

	void AddDescriptorSetBinding(const VulkanDescriptorSet& descriptorSetBinding);
	void AddDescriptorSetBindings(const Vector<VulkanDescriptorSet>& descriptorSetBindingList);
	void UpdateTextureDescriptorSet(uint32 textureGpuBufferIndex, VulkanTexture& texture, uint binding);
	void UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding);
	void BuildDescriptorSets();

	[[nodiscard]] VkDescriptorSet		  DescriptorSet()		  const noexcept;
	[[nodiscard]] VkDescriptorSetLayout   DescriptorSetLayout()   const noexcept;
	[[nodiscard]] VkDescriptorPool        DescriptorPool()        const noexcept;
};

