#include "VulkanPipeline.h"
#include "VulkanSystem.h"
#include <unordered_set>

VulkanPipeline::VulkanPipeline()
{
}

VulkanPipeline::~VulkanPipeline()
{
}

void VulkanPipeline::BuildPipelines(VulkanPipelineLoader& pipelineLoader)
{
    ShaderToPipelineBindings(pipelineLoader.VulkanShaderList);
    if (pipelineLoader.UseGlobalBindlessSet)
    {
        CreateMemoryPoolDescriptorSets(pipelineLoader);
    }
    else
    {
        CreatePipelineDescriptorSetLayout(pipelineLoader);
        AllocatePipelineDescriptorSets(pipelineLoader);
        UpdatePipelineDescriptorSets(pipelineLoader);
    }
    CreatePipelineLayout(pipelineLoader);
    CreatePipeline(pipelineLoader);
}

void VulkanPipeline::Destroy()
{
    //VkGuid                                      m_pipelineId;
    //VkPipeline                                  m_pipeline = VK_NULL_HANDLE;
    //VkPipelineCache                             m_pipelineCache = VK_NULL_HANDLE;
    //VkPipelineLayout                            m_pipelineLayout = VK_NULL_HANDLE;
    //VkDescriptorPool							m_globalBindlessPool = VK_NULL_HANDLE;
    //m_descriptorSetLayoutList = Vector<VkDescriptorSetLayout>();
    //m_descriptorSetList = Vector<VkDescriptorSet>();
    //m_pushConstantList;
    //m_vertexInputAttributeList;
    //Vector<VkVertexInputBindingDescription>     m_vertexInputBindingList;
    //Vector<ShaderDescriptorBinding>             m_descriptorBindingList;
}

void VulkanPipeline::ShaderToPipelineBindings(Vector<VulkanShader>& pipelineShaderList)
{
    std::unordered_set<uint32> uniqueSets;
    for (auto& shader : pipelineShaderList)
    {
        if (!shader.PushConstant().PushConstantName.empty())
        {
            auto it = std::ranges::find_if(m_pushConstantList, [&](const auto& existing) { return existing.PushConstantName == shader.PushConstant().PushConstantName; });
            if (it != m_pushConstantList.end()) it->ShaderStageFlags |= shader.PushConstant().ShaderStageFlags;
            else m_pushConstantList.push_back(shader.PushConstant());
        }
        
        if (shader.ShaderStages() == VK_SHADER_STAGE_VERTEX_BIT)
        {
            m_vertexInputBindingList = shader.VertexInputBindingList();
            m_vertexInputAttributeList = shader.InputVertexAttributeList();
        }
        if (shader.ShaderStages() == VK_SHADER_STAGE_VERTEX_BIT ||
            shader.ShaderStages() == VK_SHADER_STAGE_FRAGMENT_BIT)
        {
            for (const auto& descriptorSet : shader.DescriptorBindingList())
            {
                if (descriptorSet.DescriptorSet != UINT32_MAX) uniqueSets.insert(descriptorSet.DescriptorSet);

                auto it = std::ranges::find(m_descriptorBindingList, String(descriptorSet.Name), &ShaderDescriptorBinding::Name);
                if (it == m_descriptorBindingList.end()) m_descriptorBindingList.emplace_back(descriptorSet);
                else it->ShaderStageFlags |= static_cast<VkShaderStageFlags>(descriptorSet.ShaderStageFlags);
            }
            std::sort(m_descriptorBindingList.begin(), m_descriptorBindingList.end(), [](const ShaderDescriptorBinding& a, const ShaderDescriptorBinding& b)
                {
                    return a.Binding < b.Binding;
                });
        }
    }
}

void VulkanPipeline::CreateMemoryPoolDescriptorSets(VulkanPipelineLoader& pipelineLoader)
{
    Vector<VkDescriptorSet>       descriptorSetList;
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList;

    m_descriptorSetList.emplace_back(pipelineLoader.GlobalBindlessDescriptorSet);
    m_descriptorSetLayoutList.emplace_back(pipelineLoader.GlobalBindlessDescriptorSetLayout);

    std::unordered_set<uint32> uniqueSets;
    for (const auto& descriptorSet : m_descriptorBindingList)
    {
        if (descriptorSet.DescriptorSet != UINT32_MAX) uniqueSets.insert(descriptorSet.DescriptorSet);
    }
    size_t uniqueDescriptorSetCount = uniqueSets.size();

    Vector<Vector<ShaderDescriptorBinding>> descriptorSetLists;
    descriptorSetLists.resize(uniqueDescriptorSetCount);
    if (uniqueDescriptorSetCount > 1)
    {
        for (auto& descriptorSet : m_descriptorBindingList)
        {
            descriptorSetLists[descriptorSet.DescriptorSet].emplace_back(descriptorSet);
        }

        //set 0 = global descriptor set
        for (int x = 1; x < descriptorSetLists.size(); x++)
        {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            Vector<VkDescriptorSetLayoutBinding> descriptorSetBindingList;
            for (int y = 0; y < descriptorSetLists[x].size(); y++)
            {
                descriptorSetBindingList.emplace_back(VkDescriptorSetLayoutBinding
                    {
                        .binding = descriptorSetLists[x][y].Binding,
                        .descriptorType = descriptorSetLists[x][y].DescripterType,
                        .descriptorCount = 1,
                        .stageFlags = descriptorSetLists[x][y].ShaderStageFlags,
                        .pImmutableSamplers = nullptr
                    });
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo =
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                .bindingCount = static_cast<uint32>(descriptorSetBindingList.size()),
                .pBindings = descriptorSetBindingList.data()
            };
            VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkan.LogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout));

            VkDescriptorSetAllocateInfo allocInfo =
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = pipelineLoader.GlobalBindlessPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &descriptorSetLayout
            };
            VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkan.LogicalDevice(), &allocInfo, &descriptorSet));

            Vector<VkDescriptorImageInfo> subpassInputInfo = pipelineLoader.RenderPassInputTextures;
            if (subpassInputInfo.size() > descriptorSetBindingList.size()) subpassInputInfo.resize(descriptorSetBindingList.size());

            Vector<VkWriteDescriptorSet> writeDescriptorSetList;
            for (uint32 binding = 0; binding < descriptorSetBindingList.size(); ++binding)
            {
                VkDescriptorImageInfo* pInfo = binding < subpassInputInfo.size() ? &subpassInputInfo[binding] : nullptr;
                writeDescriptorSetList.push_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = descriptorSetLists[x][binding].DescripterType,
                        .pImageInfo = &subpassInputInfo[binding],
                        .pTexelBufferView = nullptr
                    });
            }
            vkUpdateDescriptorSets(vulkan.LogicalDevice(), static_cast<uint32>(writeDescriptorSetList.size()), writeDescriptorSetList.data(), 0, nullptr);
            m_descriptorSetList.emplace_back(descriptorSet);
            m_descriptorSetLayoutList.emplace_back(descriptorSetLayout);
        }
    }
}

void VulkanPipeline::CreatePipelineDescriptorSetLayout(VulkanPipelineLoader& pipelineLoader)
{
    std::unordered_set<int> uniqueValues;
    std::for_each(m_descriptorBindingList.begin(), m_descriptorBindingList.end(), [&](const ShaderDescriptorBinding& binding) { uniqueValues.insert(binding.DescriptorSet); });
    size_t countDistinct = uniqueValues.size();

    Vector<Vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindingList = Vector<Vector<VkDescriptorSetLayoutBinding>>(countDistinct);
    for (auto& descriptorBinding : m_descriptorBindingList)
    {
        descriptorSetLayoutBindingList[descriptorBinding.DescriptorSet].emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = descriptorBinding.Binding,
                .descriptorType = descriptorBinding.DescripterType,
                .descriptorCount = static_cast<uint32>(descriptorBinding.DescriptorCount),
                .stageFlags = descriptorBinding.ShaderStageFlags,
                .pImmutableSamplers = nullptr
            });
    }

    Vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfoList = Vector<VkDescriptorSetLayoutCreateInfo>(countDistinct);
    for (int x = 0; x < descriptorSetLayoutCreateInfoList.size(); x++)
    {
        descriptorSetLayoutCreateInfoList[x] = VkDescriptorSetLayoutCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV,
            .bindingCount = static_cast<uint32>(descriptorSetLayoutBindingList[x].size()),
            .pBindings = descriptorSetLayoutBindingList[x].data()
        };
    }

    Vector<VkDescriptorSetLayout> m_descriptorSetLayoutList = Vector<VkDescriptorSetLayout>(descriptorSetLayoutCreateInfoList.size());
    for (int x = 0; x < m_descriptorSetLayoutList.size(); x++)
    {
        vkCreateDescriptorSetLayout(vulkan.LogicalDevice(), &descriptorSetLayoutCreateInfoList[x], nullptr, &m_descriptorSetLayoutList[x]);
    }
}

void VulkanPipeline::AllocatePipelineDescriptorSets(VulkanPipelineLoader& pipelineLoader)
{
    for (int x = 0; x < m_descriptorSetLayoutList.size(); x++)
    {
        Vector<VkWriteDescriptorSet> writeDescriptorSet = Vector<VkWriteDescriptorSet>();
        for (auto& descriptorSetBinding : m_descriptorBindingList)
        {
            if (descriptorSetBinding.DescriptorSet != x) continue;
            writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = m_descriptorSetList[x],
                    .dstBinding = descriptorSetBinding.Binding,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<uint32>(descriptorSetBinding.DescriptorCount),
                    .descriptorType = descriptorSetBinding.DescripterType,
                    .pImageInfo = descriptorSetBinding.DescriptorImageInfo.data(),
                    .pBufferInfo = descriptorSetBinding.DescriptorBufferInfo.data(),
                    .pTexelBufferView = nullptr
                });
        }
        vkUpdateDescriptorSets(vulkan.LogicalDevice(), static_cast<uint32>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
    }
}

void VulkanPipeline::UpdatePipelineDescriptorSets(VulkanPipelineLoader& pipelineLoader)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    for(auto& pushConstant : m_pushConstantList)
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = pushConstant.ShaderStageFlags,
                .offset = 0,
                .size = static_cast<uint>(pushConstant.PushConstantSize)
            });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32>(m_descriptorSetLayoutList.size()),
        .pSetLayouts = m_descriptorSetLayoutList.data(),
        .pushConstantRangeCount = static_cast<uint32>(pushConstantRangeList.size()),
        .pPushConstantRanges = pushConstantRangeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreatePipelineLayout(vulkan.LogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));
}

void VulkanPipeline::CreatePipelineLayout(VulkanPipelineLoader& pipelineLoader)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    for (auto& pushConstant : m_pushConstantList)
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = pushConstant.ShaderStageFlags,
                .offset = 0,
                .size = static_cast<uint>(pushConstant.PushConstantSize)
            });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32>(m_descriptorSetLayoutList.size()),
        .pSetLayouts = m_descriptorSetLayoutList.data(),
        .pushConstantRangeCount = static_cast<uint32>(pushConstantRangeList.size()),
        .pPushConstantRanges = pushConstantRangeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreatePipelineLayout(vulkan.LogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
}

void VulkanPipeline::CreatePipeline(VulkanPipelineLoader& pipelineLoader)
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkPipelineVertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint>(m_vertexInputBindingList.size()),
        .pVertexBindingDescriptions = m_vertexInputBindingList.data(),
        .vertexAttributeDescriptionCount = static_cast<uint>(m_vertexInputAttributeList.size()),
        .pVertexAttributeDescriptions = m_vertexInputAttributeList.data()
    };

    for (auto& viewPort : pipelineLoader.ViewportList)
    {
        viewPort.width = static_cast<float>(pipelineLoader.RenderPassResolution.x);
        viewPort.height = static_cast<float>(pipelineLoader.RenderPassResolution.y);
    }

    for (auto& scissor : pipelineLoader.ScissorList)
    {
        scissor.extent.width = static_cast<float>(pipelineLoader.RenderPassResolution.x);
        scissor.extent.height = static_cast<float>(pipelineLoader.RenderPassResolution.y);
    }

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = static_cast<uint32>(pipelineLoader.ViewportList.size() ? pipelineLoader.ViewportList.size() : 1),
        .pViewports = pipelineLoader.ViewportList.data(),
        .scissorCount = static_cast<uint32>(pipelineLoader.ScissorList.size() ? pipelineLoader.ScissorList.size() : 1),
        .pScissors = pipelineLoader.ScissorList.data()
    };

    Vector<VkDynamicState> dynamicStateList;
    if (pipelineLoader.ViewportList.empty() || pipelineLoader.ScissorList.empty())
    {
        dynamicStateList.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStateList.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32>(dynamicStateList.size()),
        .pDynamicStates = dynamicStateList.data()
    };

    Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList;
    for (auto& shader : pipelineLoader.VulkanShaderList)
    {
        pipelineShaderStageCreateInfoList.emplace_back(shader.GetShader());
    }

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = pipelineLoader.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = pipelineLoader.PipelineColorBlendAttachmentStateList.size();
    pipelineColorBlendStateCreateInfoModel.pAttachments = pipelineLoader.PipelineColorBlendAttachmentStateList.data();

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = pipelineLoader.PipelineMultisampleStateCreateInfo;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = pipelineMultisampleStateCreateInfo.rasterizationSamples >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : pipelineMultisampleStateCreateInfo.rasterizationSamples;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = pipelineMultisampleStateCreateInfo.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = VkGraphicsPipelineCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32>(pipelineShaderStageCreateInfoList.size()),
        .pStages = pipelineShaderStageCreateInfoList.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &pipelineLoader.PipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &pipelineLoader.PipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = &pipelineLoader.PipelineDepthStencilStateCreateInfo,
        .pColorBlendState = &pipelineColorBlendStateCreateInfoModel,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = pipelineLoader.RenderPass,
        .subpass = pipelineLoader.SubPassId,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
    };
    VULKAN_THROW_IF_FAIL(vkCreateGraphicsPipelines(vulkan.LogicalDevice(), m_pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &m_pipeline));
}

VkPipeline                    VulkanPipeline::Pipeline()                const { return m_pipeline; }
VkPipelineCache               VulkanPipeline::PipelineCache()           const { return m_pipelineCache; }
VkPipelineLayout              VulkanPipeline::PipelineLayout()          const { return m_pipelineLayout; }
Vector<VkDescriptorSetLayout> VulkanPipeline::DescriptorSetLayoutList() const { return m_descriptorSetLayoutList; }
Vector<VkDescriptorSet>       VulkanPipeline::DescriptorSetList()       const { return m_descriptorSetList; }
